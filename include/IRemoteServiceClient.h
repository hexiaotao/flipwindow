/*
 * native common.
 *
 */

#ifndef IREMOTESERVICECLIENT_H
#define IREMOTESERVICECLIENT_H

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>

namespace android {

class IRemoteServiceClient : public IInterface
{
public:
	DECLARE_META_INTERFACE(RemoteServiceClient);

	//
	// IRemoteServiceClient interface.
	//
	virtual status_t onDisplayEvent(int displayId, int event) = 0;
};


// ----------------------------------------------------------------------------

class BnRemoteServiceClient : public BnInterface<IRemoteServiceClient>
{
public:
	virtual status_t    onTransact(uint32_t code,
					const Parcel& data,
					Parcel* reply,
					uint32_t flags = 0);
};

// ----------------------------------------------------------------------------

};

#endif
