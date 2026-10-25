modded class PlayerBase
{
    //! server: time of the last "gas tank is empty" notification (no spam when the request repeats)
    int m_GTR_LastEmptyNotice;

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

        if (rpc_type == GasTankRefillConfig.RPC_SYNC_CONFIG)
        {
            GasTankRefillConfig.OnReceiveConfig(ctx);
        }
    }
};
