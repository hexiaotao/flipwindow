/*
 * native common.
 *
 */

#define LOG_TAG "IRemoteServiceClient"
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>

#include <IRemoteServiceClient.h>

namespace android {

enum {
	ON_DISPLAY_EVENT = IBinder::FIRST_CALL_TRANSACTION,
};

class BpRemoteServiceClient : public BpInterface<IRemoteServiceClient>
{
public:
	BpRemoteServiceClient(const sp<IBinder>& impl)
		: BpInterface<IRemoteServiceClient>(impl)
	{
	}

	virtual status_t onDisplayEvent(const int displayId, const int event)
	{
		Parcel data, reply;
		data.writeInterfaceToken(IRemoteServiceClient::getInterfaceDescriptor());
		data.writeInt32((int32_t)displayId);
		data.writeInt32((int32_t)event);
		return remote()->transact(ON_DISPLAY_EVENT, data, &reply);
	}
};

IMPLEMENT_META_INTERFACE(RemoteServiceClient, "android.hardware.display.IDisplayManagerCallback");
// ----------------------------------------------------------------------

status_t BnRemoteServiceClient::onTransact(
	uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
	ALOGI("IRemoteServiceClient onTransact() code=%d", code);
	switch (code) {
	case ON_DISPLAY_EVENT: {
		CHECK_INTERFACE(IRemoteServiceClient, data, reply);
		int displayId = data.readInt32();
		int event = data.readInt32();
		onDisplayEvent(displayId, event);
		return NO_ERROR;
	} break;

	default:
		return BBinder::onTransact(code, data, reply, flags);
	}
}

};
