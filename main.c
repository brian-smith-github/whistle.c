#include <stdio.h>
#include <math.h>
#include <string.h>
#include "nd_whistle.h"

static nd_whistle s_w;

#define FLOOR_HIST     250           /* 5 s of idle history */
#define GATE_DB        12.0f         /* speech must clear the floor by this */

/* Noise floor from the last 5 s of idle audio: the 20th-percentile 20 ms
 * window. The mic runs between presses, so this is known before anyone
 * speaks - better than guessing it from a clip that may be all speech. */
static float s_floor_hist[FLOOR_HIST];
static int   s_floor_n, s_floor_i;


//---------------------------------------------------------------------------------------
static float noise_floor(void)
{
    static float tmp[FLOOR_HIST];
    int          i, j, n = s_floor_n;
    if (n < 25)
        return 0.0f;                       /* < 0.5 s seen: let the clip decide */
    memcpy(tmp, s_floor_hist, n * sizeof(float));
    for (i = 1; i < n; i++)
        for (j = i; j > 0 && tmp[j - 1] > tmp[j]; j--) {
            float t = tmp[j]; tmp[j] = tmp[j - 1]; tmp[j - 1] = t;
        }
    return tmp[n / 5];
}

/* ------------------------------------------------------------- inference */

static void transcribe(float *x, uint32_t n)
{
    uint32_t s0 = 0, s1 = n, T, ids[96], i;
    float    floor_rms = noise_floor(), peak = 0.0f, *enc;
    int      nt;
    char     text[512];
 
    for (i = 0; i < n; i++)
        if (fabsf(x[i]) > peak)
            peak = fabsf(x[i]);
    //printf("EVT clip ms=%u peak_dbfs=%.1f floor_dbfs=%.1f\n", (unsigned)(n / 16),
    //       peak > 0 ? 20.0f * log10f(peak) : -120.0f,
    //       floor_rms > 0 ? 20.0f * log10f(floor_rms) : -120.0f);

    if (!nd_whistle_find_speech(x, n, floor_rms, GATE_DB, 300, &s0, &s1)) {
        printf("EVT no_speech (nothing %.0f dB above the noise floor)\n", GATE_DB);
        printf("TXT \nACT none\n");    
        return;
    }
    x += s0;
    n  = s1 - s0;
    T  = nd_whistle_frames(&s_w, n);
    //printf("EVT speech ms=%u frames=%u\n", (unsigned)(n / 16), (unsigned)T);
     
    memset(nd_wprof, 0, sizeof(nd_wprof));
    enc = (float *)malloc((size_t)T * s_w.d * sizeof(float));
    
    if (!enc || nd_whistle_encode(&s_w, x, n, enc) != 0) {
        printf("ERR encode\n");      
        return;
    }
   
    nt = nd_whistle_decode(&s_w, enc, T, -1 /* detect */, ids, sizeof(ids) / sizeof(ids[0]));
    if (nt < 0) {
        printf("ERR decode\n");
        return;
    }
    nd_whistle_detok(&s_w, ids, (uint32_t)nt, text, sizeof(text));

    printf("%s\n", text);
}

//-----------------------------------------------------------------------

void main()
{
  FILE *fp;
  int i,blobsize,clipsize,z,rc;
  void *blob; // holds the whistle.cact file in memory
  short *clip16; // the clip.wav file as raw PCM 16-bit 16000 samples/sec
  float *clip; // the clip as (-1:1) 16000 samples/sec floating-point
  
  printf("transcribing...\n");
  
  // read in the model...
  fp=fopen("whistle.cact","r");
  fseek(fp,0,SEEK_END); blobsize=ftell(fp); rewind(fp);
  blob=malloc(blobsize); z=fread(blob,blobsize,1,fp); fclose(fp);
  
  rc = nd_whistle_open(&s_w, blob, blobsize);

  // read in audio as 16-bit PCM....
  fp=fopen("clip.raw","r");
  fseek(fp,0,SEEK_END); clipsize=ftell(fp); clipsize/=2; rewind(fp);
  clip16=(short *)malloc(clipsize*2);
  z=fread(clip16,clipsize,2,fp); fclose(fp);
  
  // convert audio to an array of float in range -1.0 to 1.0
  clip=(float *)malloc(clipsize*4);
  for (i=0; i<clipsize; i++) {clip[i]=(float)clip16[i];  clip[i]/=32768.0; }
 
  // run the transcription process and output result.
  transcribe(clip,clipsize);
  
}
