/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2021 Live Networks, Inc.  All rights reserved.
// A file source that is a plain byte stream (rather than frames)
// Implementation

#include "ByteStreamLiveSource.hh"
#include "GroupsockHelper.hh"

#include <iostream>
using namespace std;
////////// ByteStreamLiveSource //////////

ByteStreamLiveSource*
ByteStreamLiveSource::createNew(UsageEnvironment& env, GetFrameCB funcCb,int chId,int srcId,
				unsigned preferredFrameSize,
				unsigned playTimePerFrame) {

  ByteStreamLiveSource* newSource
    = new ByteStreamLiveSource(env, chId, srcId,preferredFrameSize, playTimePerFrame);
  newSource->getFrameFunc = funcCb;

  return newSource;
}

ByteStreamLiveSource::ByteStreamLiveSource(UsageEnvironment& env, int mchId,int msrcId,
					   unsigned preferredFrameSize,
					   unsigned playTimePerFrame)
  : FramedSource(env), chId(mchId),srcId(msrcId), fPreferredFrameSize(preferredFrameSize),
    fPlayTimePerFrame(playTimePerFrame), fLastPlayTime(0),
    fHaveStartedReading(False), fLimitNumBytesToStream(False), fNumBytesToStream(0) {

}

ByteStreamLiveSource::~ByteStreamLiveSource() {

}

void ByteStreamLiveSource::doGetNextFrame() {
  if ((fLimitNumBytesToStream && fNumBytesToStream == 0)) {
    handleClosure();
    return;
  }

  doReadFromFile();
}

void ByteStreamLiveSource::doStopGettingFrames() {
  envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void ByteStreamLiveSource::getFrameableHandler(ByteStreamLiveSource* source, int /*mask*/) {
  if (!source->isCurrentlyAwaitingData()) {
    source->doStopGettingFrames(); // we're not ready for the data yet
    return;
  }
  source->doReadFromFile();
}

void ByteStreamLiveSource::doReadFromFile() {
  // Try to read as many bytes as will fit in the buffer provided (or "fPreferredFrameSize" if less)
  if (fLimitNumBytesToStream && fNumBytesToStream < (u_int64_t)fMaxSize) {
    fMaxSize = (unsigned)fNumBytesToStream;
  }
  if (fPreferredFrameSize > 0 && fPreferredFrameSize < fMaxSize) {
    fMaxSize = fPreferredFrameSize;
  }

	cout <<"famxSize: "<< fMaxSize << endl;
	cout <<"fPreferredFrameSize: " << fPreferredFrameSize << endl;
	cout <<"fNumBytesToStream: " <<fNumBytesToStream << endl;

	fFrameSize =0;
	if(getFrameFunc != NULL){
		cout <<"doGetNextFrame call back getFrameFunc" << endl;
		//callback function get encoder frame-----
		fFrameSize = getFrameFunc(chId,srcId,fTo,fMaxSize); 
	}
  cout <<"fFrameSize: "<< fFrameSize << endl;

  if (fFrameSize == 0) {
    handleClosure();
    return;
  }
  fNumBytesToStream -= fFrameSize;

  // Set the 'presentation time':
  if (fPlayTimePerFrame > 0 && fPreferredFrameSize > 0) {
    if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
      // This is the first frame, so use the current time:
      gettimeofday(&fPresentationTime, NULL);
    } else {
      // Increment by the play time of the previous data:
      unsigned uSeconds	= fPresentationTime.tv_usec + fLastPlayTime;
      fPresentationTime.tv_sec += uSeconds/1000000;
      fPresentationTime.tv_usec = uSeconds%1000000;
    }

    // Remember the play time of this data:
    fLastPlayTime = (fPlayTimePerFrame*fFrameSize)/fPreferredFrameSize;
    fDurationInMicroseconds = fLastPlayTime;
  } else {
    // We don't know a specific play time duration for this data,
    // so just record the current time as being the 'presentation time':
    gettimeofday(&fPresentationTime, NULL);
  }

  // To avoid possible infinite recursion, we need to return to the event loop to do this:
  nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
				(TaskFunc*)FramedSource::afterGetting, this);

}
