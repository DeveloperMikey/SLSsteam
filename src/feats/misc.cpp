#include "misc.hpp"

#include "../sdk/CNetPacket.hpp"
#include "../sdk/CProtoBufMsgBase.hpp"
#include "../sdk/IClientUtils.hpp"

#include "../config.hpp"

#include "fakeappid.hpp"

bool Misc::shouldFakeOffline()
{
	if (!g_pClientUtils)
	{
		return false;
	}
	
	const uint32_t appId = FakeAppIds::getRealAppIdForCurrentPipe();
	if (!appId || !g_config.fakeOffline.get().contains(appId))
	{
		return false;
	}

	g_pLog->once("Faking offline mode for %u\n", appId);
	return true;
}


void Misc::recvMsg(CNetPacket *msg)
{
	switch(msg->getProtoBufType())
	{
		case EMSG_WALLET_INFO_UPDATE:
		{
			const int32_t amount = g_config.fakeWalletBalance.get();
			if (!amount)
			{
				return;
			}

			auto body = msg->deserializeBody<CMsgClientWalletInfoUpdate>();
			body.set_has_wallet(true);
			body.set_balance(amount);
			body.set_balance64(amount);

			msg->serializeBody(body);
			break;
		}

		case EMSG_EMAIL_ADDRESS_INFO:
		{
			const auto email = g_config.fakeEmail.get();
			if (email.size() < 1)
			{
				return;
			}

			auto body = msg->deserializeBody<CMsgClientEmailAddrInfo>();
			body.set_email_address(email);
			body.set_email_is_validated(true);

			msg->serializeBody(body);
			break;
		}
	}
}
