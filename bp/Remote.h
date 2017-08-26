/*
 * bp native.
 *
 */

#ifndef REMOTE_H
#define REMOTE_H

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/Errors.h>

#include <IRemoteService.h>
#include <IRemoteServiceClient.h>

typedef void (*stateCallback_t)(int displayId, int event);

namespace android {

class Remote :
	public BnRemoteServiceClient,
	public IBinder::DeathRecipient
{
public:
	static sp<Remote> getInstance();

	static const sp<IRemoteService> get_remote_service();
	static int setListener(stateCallback_t cb);

private:
	Remote() ANDROID_API;
	virtual ~Remote();
	virtual void onFirstRef();

	virtual void binderDied(const wp<IBinder>& who);
	virtual status_t onDisplayEvent(int displayId, int event);

	static Mutex gLock;
	static sp<Remote> gRemote;

	static sp<IRemoteService> gRemoteService;
	static Mutex gLockRemote;

	static stateCallback_t gStateCallback;
};

};
#endif
