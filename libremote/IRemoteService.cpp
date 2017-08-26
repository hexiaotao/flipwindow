/*
 * native common.
 *
 */

#define LOG_TAG "IRemoteService"
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>

#include <IRemoteService.h>
#include <IRemoteServiceClient.h>

namespace android {

enum {
	GET_DISPLAY_INFO = IBinder::FIRST_CALL_TRANSACTION,
	GET_DISPLAY_IDS,
	REGISTER_CALLBACK,
};

class BpRemoteService : public BpInterface<IRemoteService>
{
public:
	BpRemoteService(const sp<IBinder>& impl)
		: BpInterface<IRemoteService>(impl)
	{
	}

	virtual int registerCallback(const sp<IRemoteServiceClient>& callback)
	{
		Parcel data, reply;
		data.writeInterfaceToken(IRemoteService::getInterfaceDescriptor());
		data.writeStrongBinder(IInterface::asBinder(callback));
		status_t status = remote()->transact(REGISTER_CALLBACK, data, &reply);
		if (status != NO_ERROR) {
			return -1;
		}

		return (int)reply.readInt32();
	}
};

IMPLEMENT_META_INTERFACE(RemoteService, "android.hardware.display.IDisplayManager");

};
