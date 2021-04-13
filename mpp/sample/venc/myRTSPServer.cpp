#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "announceURL.hh"

#include "myRTSPServer.hh"
UsageEnvironment *env;

// To make the second and subsequent client for each stream reuse the same
// input stream as the first client (rather than playing the file from the
// start for each client), change the following "False" to "True":
Boolean reuseFirstSource = False;

// To stream *only* MPEG-1 or 2 video "I" frames
// (e.g., to reduce network bandwidth),
// change the following "False" to "True":
Boolean iFramesOnly = False;

static void announceStream(RTSPServer *rtspServer, ServerMediaSession *sms,
                           char const *streamName, char const *inputFileName)
{
  UsageEnvironment &env = rtspServer->envir();

  env << "\n\"" << streamName << "\" stream, from the file \""
      << inputFileName << "\"\n";
  announceURL(rtspServer, sms);
}

static int getTestFrame(int chId,int srcId,unsigned char* buf,int size)
{
  return 0;
}
// #define ACCESS_CONTROL
int main(int argc, char **argv)
{
  if(SAMPLE_VENC_H265_H264(1))
    return -1;
  // Begin by setting up our usage environment:
  TaskScheduler *scheduler = BasicTaskScheduler::createNew(5000);
  env = BasicUsageEnvironment::createNew(*scheduler);

  UserAuthenticationDatabase *authDB = NULL;

#ifdef ACCESS_CONTROL
  // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("ppq", "123"); // replace these with real strings
  // Repeat the above with each <username>, <password> that you wish to allow
  // access to the server.
#endif

  // Create the RTSP server:
  RTSPServer *rtspServer = RTSPServer::createNew(*env, 8554, authDB);
  if (rtspServer == NULL)
  {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }
  // OutPacketBuffer::maxSize = 5000000;
  char const *descriptionString = "Session streamed by \"testOnDemandRTSPServer\"";

  // Set up each of the possible streams that can be served by the
  // RTSP server.  Each such stream is implemented using a
  // "ServerMediaSession" object, plus one or more
  // "ServerMediaSubsession" objects for each audio/video substream.

  // A MPEG-4 video elementary stream:
  {
    char const *streamName = "mpeg4ESVideoTest";
    char const *inputFileName = "test.m4e";
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                                            descriptionString);
    sms->addSubsession(MPEG4VideoFileServerMediaSubsession ::createNew(*env, inputFileName, reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A H.264 video elementary stream:
  {
    char const *streamName = "h264ESVideoTest";
    char const *inputFileName = "test.264";
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                                            descriptionString);
    sms->addSubsession(H264VideoFileServerMediaSubsession ::createNew(*env, inputFileName, reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A H.264 live video elementary stream:
  {
    char const *streamName = "h264Live";
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                                            descriptionString);
    sms->addSubsession(H264VideoLiveServerMediaSubsession::createNew(*env, getVencFrame,0,0,reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, "hisih264Live");
  }

  {
    char const *streamName = "h265Live";
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                                            descriptionString);
    sms->addSubsession(H265VideoLiveServerMediaSubsession::createNew(*env, getVencFrame,1,0,reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, "hisih265Live");
  }

  // A H.265 video elementary stream:
  {
    char const *streamName = "h265ESVideoTest";
    char const *inputFileName = "test.265";
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                                            descriptionString);
    sms->addSubsession(H265VideoFileServerMediaSubsession ::createNew(*env, inputFileName, reuseFirstSource));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A MPEG-1 or 2 audio+video program stream:
  {
    char const *streamName = "mpeg1or2AudioVideoTest";
    char const *inputFileName = "test.mpg";
    // NOTE: This *must* be a Program Stream; not an Elementary Stream
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                                            descriptionString);
    MPEG1or2FileServerDemux *demux = MPEG1or2FileServerDemux::createNew(*env, inputFileName, reuseFirstSource);
    sms->addSubsession(demux->newVideoServerMediaSubsession(iFramesOnly));
    sms->addSubsession(demux->newAudioServerMediaSubsession());
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A MPEG-1 or 2 video elementary stream:
  {
    char const *streamName = "mpeg1or2ESVideoTest";
    char const *inputFileName = "testv.mpg";
    // NOTE: This *must* be a Video Elementary Stream; not a Program Stream
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                                            descriptionString);
    sms->addSubsession(MPEG1or2VideoFileServerMediaSubsession ::createNew(*env, inputFileName, reuseFirstSource, iFramesOnly));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A MP3 audio stream (actually, any MPEG-1 or 2 audio file will work):
  // To stream using 'ADUs' rather than raw MP3 frames, uncomment the following:
  //#define STREAM_USING_ADUS 1
  // To also reorder ADUs before streaming, uncomment the following:
  //#define INTERLEAVE_ADUS 1
  // (For more information about ADUs and interleaving,
  //  see <http://www.live555.com/rtp-mp3/>)
  {
    char const *streamName = "mp3AudioTest";
    char const *inputFileName = "test.mp3";
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                                            descriptionString);
    Boolean useADUs = False;
    Interleaving *interleaving = NULL;
#ifdef STREAM_USING_ADUS
    useADUs = True;
#ifdef INTERLEAVE_ADUS
    unsigned char interleaveCycle[] = {0, 2, 1, 3}; // or choose your own...
    unsigned const interleaveCycleSize = (sizeof interleaveCycle) / (sizeof(unsigned char));
    interleaving = new Interleaving(interleaveCycleSize, interleaveCycle);
#endif
#endif
    sms->addSubsession(MP3AudioFileServerMediaSubsession ::createNew(*env, inputFileName, reuseFirstSource,
                                                                     useADUs, interleaving));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // A WAV audio stream:
  {
    char const *streamName = "wavAudioTest";
    char const *inputFileName = "test.wav";
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, streamName, streamName,
                                                            descriptionString);
    // To convert 16-bit PCM data to 8-bit u-law, prior to streaming,
    // change the following to True:
    Boolean convertToULaw = False;
    sms->addSubsession(WAVAudioFileServerMediaSubsession ::createNew(*env, inputFileName, reuseFirstSource, convertToULaw));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, streamName, inputFileName);
  }

  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).

  if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080))
  {
    *env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
  }
  else
  {
    *env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
  }

  env->taskScheduler().doEventLoop(); // does not return
  SAMPLE_VENC_H265_H264(0);
  return 0; // only to prevent compiler warning
}
