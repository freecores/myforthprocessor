/*
 * @(#)HAE_API_WinOS_Capture.c	1.23 03/05/01
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
**	HAE_API_WinOS_Capture.c
**
**	This provides platform specfic functions for Windows 95/NT. This interface
**	for HAE is for Windows 95/NT and uses the waveIn API to capture audio
**	buffer slices through the multimedia system.
**
**	Overview:
**
**	History	-
**	6/19/98		Created
**	7/23/98		Added a new parameter to HAE_AquireAudioCapture
**				Added HAE_PauseAudioCapture & HAE_ResumeAudioCapture
**	8/3/98		Added support for multi devices and control which device is active
**				Fixed some type casting
**
**	JAVASOFT
**	10.14.98	$$kk: messed with this file utterly.  changed callback mechanism to
**				run a separate thread for callbacks (analogous with output) and not
**				use windows callbacks: 1) dangerous from java because you can't pass
**				JNIEnv pointers between threads and 2) on WinNT, waveInAddBuffer called
**				on windows callback thread locks forever.
**	03.31.99:	$$kk: fixed bug in HAE_GetCaptureDeviceName that causes a crash.
**  06.28.99:   $$jb: merged in changes to provide dynamic capture buffer size,
**              changes API for HAE_AquireAudioCapture
**
**	10.27.99:	$$kk: added HAE_MaxCaptureFormats and HAE_GetSupportedCaptureFormats.
**				added encoding parameter to HAE_AquireAudioCapture.
**
*/
/*****************************************************************************/

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <mmsystem.h>
#include <windowsx.h>

#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include "HAE_API.h"

#ifndef USE_DIRECTSOUND
#define USE_DIRECTSOUND				1
#endif

#if USE_DIRECTSOUND
#include <dsound.h>
#endif


#define HAE_WAVEIN_NUM_BUFFERS				3			// number of capture buffers
#define HAE_WAVEIN_SOUND_PERIOD				11			// sleep period between position checks (in ms)
// $$jb 03.22.99: Changing buffer size from 16384 to 1024
// $$jb 05.10.99: Upping buffer size to 4096.  1024 is way too small
#define HAE_WAVEIN_FRAMES_PER_BUFFER		4096		// how many frames of audio data per capture buffer?
// $$kk: 10.13.98: need to decide whether / how to let user configure this;
// what is a good default??

#define HAE_WAVEIN_DEFAULT_BUFFERSIZE_IN_MS		150		// default buffer size in ms
#define HAE_WAVEIN_MIN_BUFFERSIZE_IN_MS			50		// minimum buffer size in ms


static void				*g_audioBufferBlock[HAE_WAVEIN_NUM_BUFFERS];		// actual data buffers
static INT32				g_audioBytesPerBuffer;								// size of each audio buffer in bytes

static HWAVEIN			g_captureSound = NULL;			// the capture device
static BOOL				g_captureShutdown;				// false if capture active, otherwise true

static INT32				g_audioFramesPerBuffer;			// size in sample frames of each capture buffer
// $$kk: 10.13.98: right now this is fixed as HAE_WAVEIN_FRAMES_PER_BUFFER
// $$kk: 10.13.98: need to figure out how to configure this

static short int		g_bitSize;						// bit size of current capture format
static short int		g_channels;						// number of channels in current capture format
static short int		g_sampleRate;					// sample rate of current capture format

static UINT32	g_encoding;						// audio encoding

static short int		g_soundDeviceIndex = 0;			// if non zero then use this device to open

static HAE_CaptureDone	g_captureDoneProc;
static void				*g_captureDoneContext;
static BOOL g_activeWaveInThread = FALSE;

#ifndef TRACE0
#ifdef DEBUG
#define TRACE0(text) printf(text)
#else
#define TRACE0(text)
#endif
#endif

/*
static void CALLBACK PV_AudioCaptureCallback(HWAVEIN captureHandle, UINT message,
											DWORD platformContext, DWORD dwParam1, DWORD dwParam2)
{
	dwParam1;
	dwParam2;
	platformContext;
	switch (message)
	{
		case WIM_OPEN:

			if (g_captureDoneProc)
			{
				(*g_captureDoneProc)(g_captureDoneContext, OPEN_CAPTURE, NULL, NULL);
			}

			  break;
		case WIM_CLOSE:

			if (g_captureDoneProc)
			{
				(*g_captureDoneProc)(g_captureDoneContext, CLOSE_CAPTURE, NULL, NULL);
			}

			break;
		case WIM_DATA:

			fprintf(stderr, "\nWIM_DATA\n");
			break;

			if ((g_captureShutdown == FALSE) && (g_captureSound == captureHandle))
			{
				LPWAVEHDR hdr = (LPWAVEHDR)dwParam1;

				fprintf(stderr, "\nWIM_DATA (1)\n");

				if (g_captureDoneProc)
				{
					// $$kk: 10.08.98: added check for bytes recorded in LPWAVEHDR
					//(*g_captureDoneProc)(g_captureDoneContext, DATA_READY_CAPTURE, &g_captureBuffer, &g_captureBufferSize);

					DWORD dwBytesRecorded = hdr->dwBytesRecorded;
					LPSTR lpData = hdr->lpData;

					fprintf(stderr, "\nWIM_DATA (2)\n");

					(*g_captureDoneProc)(g_captureDoneContext, DATA_READY_CAPTURE, &lpData, (void *)&dwBytesRecorded);
				}
			}
			break;
	}
}
*/


// this is run by a java thread; the context needs to be the JNI environment
// pointer valid for the thread.
void PV_AudioWaveInFrameThread(void* context)
{
    WAVEHDR			waveHeader[HAE_WAVEIN_NUM_BUFFERS];

    long			count, framesToRead, bytesToRead, error;

    long			waveHeaderCount;	// current index in the array of waveheaders
    LPWAVEHDR		pCurrentWaveHdr;

    TRACE0("> PV_AudioWaveInFrameThread\n");

    g_activeWaveInThread = TRUE;
    
    bytesToRead = g_audioBytesPerBuffer;
    framesToRead = g_audioFramesPerBuffer;

    memset(&waveHeader, 0, sizeof(WAVEHDR) * HAE_WAVEIN_NUM_BUFFERS);
    
    /* get the current position */
    /* MMTIME audioStatus;
	long currentPos;
	long lastPos;
	
	memset(&audioStatus, 0, (INT32)sizeof(MMTIME));
	audioStatus.wType = TIME_BYTES;	// get byte position
	error = waveInGetPosition(g_captureSound, &audioStatus, sizeof(MMTIME));
	currentPos = audioStatus.u.cb;
	lastPos = currentPos + g_audioBytesPerBuffer;
    */

    // set up all the capture buffers
    for (count = 0; count < HAE_WAVEIN_NUM_BUFFERS; count++)
	{
	    waveHeader[count].lpData = (char *)g_audioBufferBlock[count];
	    waveHeader[count].dwBufferLength = g_audioBytesPerBuffer;
	    waveHeader[count].dwFlags 		= 0;
	    waveHeader[count].dwLoops 		= 0;
	    error = waveInPrepareHeader(g_captureSound, &waveHeader[count], (INT32)sizeof(WAVEHDR));
	}

    // add all the capture buffers
    for (count = 0; count < HAE_WAVEIN_NUM_BUFFERS; count++)
	{
	    error = waveInAddBuffer(g_captureSound, &waveHeader[count], sizeof(WAVEHDR));
	}


    // now run this loop to do the capture.
    // we wait for enough samples to be captured to fill one capture buffer,
    // callback with the captured data, and put the buffer back in the queue.

    waveHeaderCount = 0; // which buffer we're processing
    while (g_captureShutdown == FALSE) {
    TRACE0("  PV_AudioWaveInFrameThread: in loop\n");
	    // wait for the device to record enough data to fill our capture buffer

	    // this is the data buffer for the current capture buffer
	    pCurrentWaveHdr = &waveHeader[waveHeaderCount];

	    while ( (!(pCurrentWaveHdr->dwFlags & WHDR_DONE)) && (g_captureShutdown == FALSE) ) {

		//printf("  PV_AudioWaveInFrameThread: sleep\n");
		HAE_SleepFrameThread(context, HAE_WAVEIN_SOUND_PERIOD);		// in ms

		//error = waveInGetPosition(g_captureSound, &audioStatus, sizeof(MMTIME));
		//currentPos = audioStatus.u.cb;
	    }

	    //lastPos += bytesToRead;

	    // then process the captured data
	    if (pCurrentWaveHdr->dwFlags & WHDR_DONE) {
		DWORD dwBytesRecorded = pCurrentWaveHdr->dwBytesRecorded;
		LPSTR lpData = pCurrentWaveHdr->lpData;

		// callback with the captured data
		//printf("  PV_AudioWaveInFrameThread: callback\n");
		(*g_captureDoneProc)(context, DATA_READY_CAPTURE, &lpData, (void *)&dwBytesRecorded);

		// add the buffer back into the queue
		//printf("  PV_AudioWaveInFrameThread: in addBuffer\n");
		if (g_captureShutdown == FALSE) {
		    error = waveInAddBuffer(g_captureSound, pCurrentWaveHdr, sizeof(WAVEHDR));

		    // increment to the next wavehdr
		    waveHeaderCount++;
		    if (waveHeaderCount == HAE_WAVEIN_NUM_BUFFERS) {
			waveHeaderCount = 0;
		    }
		}
	    }
	} // while

    //printf("  PV_AudioWaveInFrameThread: reset\n");
    waveInReset(g_captureSound);		// stop all audio before unpreparing headers

    // unprepare headers
    for (count = 0; count < HAE_WAVEIN_NUM_BUFFERS; count++)
	{
	    error = waveInUnprepareHeader(g_captureSound, &waveHeader[count], (INT32)sizeof(WAVEHDR));
	}
    // do this here, when we can't call it anymore.
    g_captureDoneProc = NULL;
    TRACE0("< PV_AudioWaveInFrameThread\n");
    g_activeWaveInThread = FALSE;
}


// $$kk: 10.27.99: determine what formats are supported by the device

static int encodingCount = 3; static int rateCount = 8; static int channelCount = 2; static int bitCount = 2;
static UINT32 encodingArray[] = { PCM, ULAW, ALAW };
static UINT32 sampleRateArray[] = { 8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000 };
static UINT32 channelsArray[] = { 1, 2 };
static UINT32 bitsArray[] = { 8, 16 };


int HAE_MaxCaptureFormats(INT32 deviceID)
{
    // 28 * 3 formats: PCM, ULAW, and ALAW, mono and stereo, 8 and 16 bit for these sample rates:
    // 8000
    // 11025
    // 16000
    // 22050
    // 32000
    // 44100
    // 48000


	// $$kk: 11.19.99: on systems that return more than 1 for HAE_MaxCaptureDevices(),
	// we see a crash during HAE_GetSupportedCaptureFormats() for the second device.
	// apparently memory is being overwritten.  i'm not sure how this is happening.
	// multiplying by HAE_MaxCaptureDevices() gets rid of the symptoms; need to look
	// into this more later.
	//return (encodingCount * rateCount * channelCount * bitCount);
    return (encodingCount * rateCount * channelCount * bitCount * HAE_MaxCaptureDevices());
}


int HAE_GetSupportedCaptureFormats(INT32 deviceID, UINT32 *encodings, UINT32 *sampleRates, UINT32 *channels, UINT32 *bits, int maxFormats)
{
    MMRESULT		theErr;
    WAVEFORMATEX	format;

    int encodingIndex, rateIndex, channelIndex, bitIndex;
    int numSupportedFormats = 0;

    /* $$fb 2002-04-10: fix for 4514334: JavaSoundDemo Capture not works on Windows2000 with USB Port */
    if (deviceID == 0) {
	deviceID = WAVE_MAPPER;
    } else {
	deviceID--;
    }

    for (encodingIndex = 0; encodingIndex < encodingCount; encodingIndex++)
	{
	    if (encodingArray[encodingIndex] == PCM)
		{
		    format.wFormatTag = WAVE_FORMAT_PCM;
		}
	    else
		{
		    // $$kk: 11.17.99: would need to #include MMREG.H to get WAVE_FORMAT_MULAW and WAVE_FORMAT_ALAW
		    format.wFormatTag = -1;  // error
		}

	    for (rateIndex = 0; rateIndex < rateCount; rateIndex++)
		{
		    format.nSamplesPerSec = (DWORD)sampleRateArray[rateIndex];

		    for (channelIndex = 0; channelIndex < channelCount; channelIndex++)
			{
			    format.nChannels = (WORD)channelsArray[channelIndex];

			    for (bitIndex = 0; bitIndex < bitCount; bitIndex++)
				{
				    format.wBitsPerSample = (WORD)bitsArray[bitIndex];
				    format.nBlockAlign = (WORD)((format.wBitsPerSample * format.nChannels) / 8);
				    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
				    format.cbSize = 0;

				    if (format.wFormatTag == WAVE_FORMAT_PCM)
					{
					    theErr = waveInOpen(0, deviceID, &format, 0, 0, WAVE_FORMAT_QUERY);

					    if (theErr == MMSYSERR_NOERROR)
						{
						    // this format is supported!!
						    encodings[numSupportedFormats] = PCM;
						    sampleRates[numSupportedFormats] = sampleRateArray[rateIndex];
						    channels[numSupportedFormats] = channelsArray[channelIndex];
						    bits[numSupportedFormats] = bitsArray[bitIndex];

						    numSupportedFormats++;
						    if (numSupportedFormats >= maxFormats) return numSupportedFormats;
						}
					}
				}
			}
		}
	}

    return numSupportedFormats;
}



// Aquire and enabled audio card
// return 0 if ok, -1 if failed
//int HAE_AquireAudioCapture(void *context, UINT32 sampleRate, UINT32 channels, UINT32 bits,
//							UINT32 audioFramesPerBuffer, UINT32 *pCaptureHandle)
// $$fb 2002-02-01: itanium port
int HAE_AquireAudioCapture(void *context, UINT32 encoding, UINT32 sampleRate, UINT32 channels, UINT32 bits,
			   UINT32 audioFramesPerBuffer, UINT_PTR *pCaptureHandle)
{
    MMRESULT		theErr;
    WAVEINCAPS		caps;
    WAVEFORMATEX	format;
    INT32			deviceID;
    ULONG			minFramesPerBuffer;

    g_encoding = encoding;
    g_bitSize = bits;
    g_channels = channels;
    g_sampleRate = sampleRate;


    // $$jb: 05.19.99: Setting the buffer size


    minFramesPerBuffer = sampleRate * HAE_WAVEIN_MIN_BUFFERSIZE_IN_MS / 1000;

    if( audioFramesPerBuffer == 0 ) {
	audioFramesPerBuffer = sampleRate * HAE_WAVEIN_DEFAULT_BUFFERSIZE_IN_MS / 1000;
    }

    // $$kk: 08.06.99: got rid of fixed minimum buffer size
    /*
      if ( audioFramesPerBuffer >= minFramesPerBuffer ) {
      g_audioFramesPerBuffer = audioFramesPerBuffer;
      } else {
      g_audioFramesPerBuffer = minFramesPerBuffer;
      }
    */
    g_audioFramesPerBuffer = audioFramesPerBuffer;


    if (pCaptureHandle)
	{
	    *pCaptureHandle = 0L;
	}

    /* $$fb 2002-04-10: fix for 4514334: JavaSoundDemo Capture not works on Windows2000 with USB Port */
    if (g_soundDeviceIndex == 0) {
	deviceID = WAVE_MAPPER;
    } else {
	deviceID = g_soundDeviceIndex - 1;
    }

    theErr = waveInGetDevCaps(deviceID, &caps, sizeof(WAVEINCAPS));

    if (theErr == MMSYSERR_NOERROR)
	{
	    //format.wFormatTag = WAVE_FORMAT_PCM;
	    format.nSamplesPerSec = sampleRate;
	    format.wBitsPerSample = (WORD)bits;
	    format.nChannels = (WORD)channels;

	    format.nBlockAlign = (WORD)((format.wBitsPerSample * format.nChannels) / 8);
	    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	    format.cbSize = 0;

	    if (g_encoding == PCM)
		{
		    format.wFormatTag = WAVE_FORMAT_PCM;
		}
	    else
		{
		    // fprintf(stderr, "HAE_AquireAudioCapture: unsupported encoding: %d\n", g_encoding);
		    return -1;
		}
	    // need to #include MMREG.H for these
	    /*
	      if (g_encoding == ULAW)
	      {
	      format.wFormatTag = WAVE_FORMAT_ULAW;
	      }
	      else if (g_encoding == ALAW)
	      {
	      format.wFormatTag = WAVE_FORMAT_ALAW;
	      }
	    */

	    // $$fb 2002-02-01: itanium port: use UINT_PTR for casting a pointer to UINT
	    theErr = waveInOpen(&g_captureSound, deviceID, &format,
				0L/*(DWORD)PV_AudioCaptureCallback*/, (UINT_PTR)context, CALLBACK_NULL /*CALLBACK_FUNCTION*/);

	    if (theErr == MMSYSERR_NOERROR)
		{
		    g_captureShutdown = FALSE;

		    if (pCaptureHandle)
			{
			    *pCaptureHandle = (UINT_PTR)g_captureSound;
			}
		}
	    else
		{
		    HAE_ReleaseAudioCapture(context);
		}
	}

    return (theErr == MMSYSERR_NOERROR) ? 0 : -1;
}

// Given the capture hardware is working, fill a buffer with some data. This call is
// asynchronous. When this buffer is filled, the function done will be called.
// returns 0 for success, -1 for failure
//int HAE_StartAudioCapture(void *buffer, UINT32 bufferSize, HAE_CaptureDone done, void *callbackContext)
int HAE_StartAudioCapture(HAE_CaptureDone done, void *callbackContext)
{
    long error = 0;
    int i;

    // start capture
    g_captureDoneProc = done;
    g_captureDoneContext = callbackContext;

    // our capture buffer will hold this many frames of sampled audio data

    //if( g_audioFramesPerBuffer < HAE_WAVEIN_FRAMES_PER_BUFFER ) {
    //	g_audioFramesPerBuffer = HAE_WAVEIN_FRAMES_PER_BUFFER;
    //}


    // calculate the number of bytes per capture buffer
    if (g_bitSize == 8)
	{
	    g_audioBytesPerBuffer = (sizeof(char) * g_audioFramesPerBuffer);
	}
    else
	{
	    g_audioBytesPerBuffer = (sizeof(short int) * g_audioFramesPerBuffer);
	}
    g_audioBytesPerBuffer *= g_channels;

    // allocate the capture data buffers
    // this could really be done in initialiation...?
    for (i = 0; i < HAE_WAVEIN_NUM_BUFFERS; i++)
	{
	    g_audioBufferBlock[i] = HAE_Allocate(g_audioBytesPerBuffer);
	    if (g_audioBufferBlock[i] == NULL)
		{
		    error = -1;	// something is wrong
		    break;
		}
	} // for

    if (error == 0)
	{
	    // create thread to manage audio capture
	    error = HAE_CreateFrameThread(callbackContext, PV_AudioWaveInFrameThread);

	    if (error == 0)
		{
		    error = HAE_ResumeAudioCapture();
		}
	}

    if (error == 0)
	{
	    // $$kk: 10.12.98: added this so we can restart capture
	    g_captureShutdown = FALSE;
	}

    return (error == 0) ? 0 : -1;
}


// stop the capture hardware
int HAE_StopAudioCapture(void* context)
{
    MMRESULT	theErr;
    int i;

    TRACE0("> HAE_StopAudioCapture\n");
    if (g_captureSound)
	{
	    // tell the thread to die
	    // the thread will also reset the device
	    g_captureShutdown = TRUE;

	    // stop streaming data
	    theErr = HAE_PauseAudioCapture();

	    // destroy the audio capture thread. 
    
	    /* $$fb:
	     * this is a dummy operation! It wouldn't even
	     * distinguish between playback thread and capture thread...
	     */
	    HAE_DestroyFrameThread(NULL);

	    //printf("  waiting for thread to complete\n");
	    // wait for thread to complete
	    while (g_activeWaveInThread) {
		HAE_SleepFrameThread(context, 10);
	    }

	    // deallocate the capture data buffers
	    for (i = 0; i < HAE_WAVEIN_NUM_BUFFERS; i++) {
		    HAE_Deallocate(g_audioBufferBlock[i]);
	    } // for
	}
    TRACE0("< HAE_StopAudioCapture\n");
    return 0;
}

int HAE_PauseAudioCapture(void)
{
    if (g_captureSound)
	{
	    // stop streaming data
	    waveInStop(g_captureSound);
	}
    return 0;
}

int HAE_ResumeAudioCapture(void)
{
    if (g_captureSound)
	{
	    // start streaming data
	    waveInStart(g_captureSound);
	}
    return 0;
}



// Release and free audio card.
// return 0 if ok, -1 if failed.
int HAE_ReleaseAudioCapture(void *context)
{
    TRACE0("> HAE_ReleaseAudioCapture\n");
    if (g_captureSound)
	{
	    // play it safe: destroy thread if not already done
	    if (!g_captureShutdown) {
		HAE_StopAudioCapture(context);
	    }

	    //printf("  WaveInClose\n");
	    while (waveInClose(g_captureSound) == WAVERR_STILLPLAYING)
		{
		    /*HAE_StopAudioCapture();*/
		    HAE_SleepFrameThread(context, 10); // in millis
		}
	    g_captureSound = NULL;
	}
    TRACE0("< HAE_ReleaseAudioCapture\n");
    return 0;
}


// number of devices. ie different versions of the HAE connection. DirectSound and waveOut
// return number of devices. ie 1 is one device, 2 is two devices.
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_MaxCaptureDevices(void)
{
    /* $$fb 2002-04-10: fix for 4514334: JavaSoundDemo Capture not works on Windows2000 with USB Port
     * return WAVE_MAPPER + available devices
     */
    return waveInGetNumDevs()?waveInGetNumDevs()+1:0;
}


// set the current device. device is from 0 to HAE_MaxDevices() - 1
// NOTE:	This function needs to function before any other calls may have happened.
//			Also you will need to call HAE_ReleaseAudioCard then HAE_AquireAudioCard
//			in order for the change to take place.
void HAE_SetCaptureDeviceID(INT32 deviceID, void *deviceParameter)
{
    if (deviceID < HAE_MaxCaptureDevices())
	{
	    g_soundDeviceIndex = deviceID;
	}
    deviceParameter;
}


// return current device ID
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_GetCaptureDeviceID(void *deviceParameter)
{
    deviceParameter;
    if (g_soundDeviceIndex)
	{	// if its non-zero then we've set it, so return value - 1
	    return g_soundDeviceIndex - 1;
	}
    return 1;
}


// get deviceID name
// NOTE:	This function needs to function before any other calls may have happened.
//			The names returned in this function are platform specific.
void HAE_GetCaptureDeviceName(INT32 deviceID, char *cName, UINT32 cNameLength)
{
    WAVEINCAPS		caps;
    MMRESULT		theErr;

    if (deviceID < HAE_MaxCaptureDevices())
	{
	    /* $$fb 2002-04-10: fix for 4514334: JavaSoundDemo Capture not works on Windows2000 with USB Port */
	    if (deviceID == 0) {
		deviceID = (INT32) WAVE_MAPPER;
	    } else {
		deviceID--;
	    }

	    theErr = waveInGetDevCaps((UINT)deviceID, &caps, sizeof(WAVEINCAPS));
	    if ((theErr == MMSYSERR_NOERROR) && cName && cNameLength)
		{
		    strncpy(cName, caps.szPname, cNameLength-1);

		    // $$kk: 03.31.99: this will crash!
		    // cName[cNameLength] = 0;
		    cName[cNameLength-1] = 0;
		}
	}
}


// return the number of frames in the capture buffer
// (should make this settable?)
UINT32 HAE_GetCaptureBufferSizeInFrames()
{

    return g_audioFramesPerBuffer;
}


// return the number of buffers used.
int HAE_GetCaptureBufferCount()
{
    return HAE_WAVEIN_NUM_BUFFERS;
}


// return the number of samples captured at the device
// $$kk: 10.15.98: need to implement!
UINT32 HAE_GetDeviceSamplesCapturedPosition()
{
    return 0L;
}


void HAE_FlushAudioCapture()
{
    /* not implemented */
}

// EOF of HAE_API_WinOS_Capture.c
