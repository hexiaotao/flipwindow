/*
 * native common.
 *
 */

#ifndef IREMOTESERVICE_H
#define IREMOTESERVICE_H

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>

namespace android {

class IRemoteServiceClient;

class IRemoteService : public IInterface
{
public:
	DECLARE_META_INTERFACE(RemoteService);

	//
	// IRemoteService interface.
	//
	virtual int registerCallback(const sp<IRemoteServiceClient>& callback) = 0;
};


// ----------------------------------------------------------------------------

class BnRemoteService : public BnInterface<IRemoteService>
{
public:
	virtual status_t    onTransact( uint32_t code,
					const Parcel& data,
					Parcel* reply,
					uint32_t flags = 0);
};

// ----------------------------------------------------------------------------

};

#endif
