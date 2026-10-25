modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        
        GasTankRefillConfig.Load();
        
        Print("[GasTankRefillMod] MissionServer initialized");
    }
    
    override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
    {
        super.InvokeOnConnect(player, identity);
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SendConfigDelayed, 2000, false, player);
    }
    
    void SendConfigDelayed(PlayerBase player)
    {
        if (player && player.GetIdentity())
        {
            GasTankRefillConfig.SendConfigToPlayer(player);
        }
    }
};

modded class MissionGameplay
{
    override void OnInit()
    {
        super.OnInit();
        
        if (GetGame().IsServer())
        {
            GasTankRefillConfig.Load();
        }
        
        Print("[GasTankRefillMod] MissionGameplay initialized");
    }
};
