//! Extra refill tanks for one map ("MapName" = the world name, e.g. enoch, sakhal, nasdara)
class GasTankRefillMapTanks
{
    string MapName;
    ref array<string> GasTanks = new array<string>;
};

class GasTankRefillConfig
{
    int ConfigVersion = 4;

    bool ModEnabled = true;
    bool DebugLogging = false;

    //! a canister is filled at a tank up to this percentage (1 - 100)
    float TankFillPercent = 100;
    //! 1 = normal filling time at a tank, 2 = twice as long (0.1 - 10)
    float TankFillTimeMultiplier = 1.0;
    //! how far (m) the player may stand from the tank - from the feet to the tank body (0.5 - 3.5)
    float TankReachDistance = 2.0;
    //! canisters one tank can fill until the server restarts, 0 = unlimited
    int TankRefillsPerRestart = 0;
    //! full refills of an empty lighter from one Lighter gas can (1 - 100)
    int LighterRefillsPerCan = 10;
    //! 1 = normal lighter refilling time (0.1 - 10)
    float LighterFillTimeMultiplier = 1.0;

    //! refill tanks on every map: part of the model path (or a class name) of a map object
    ref array<string> AllowedGasTanks = new array<string>;
    //! extra refill tanks used only on the named map
    ref array<ref GasTankRefillMapTanks> MapGasTanks = new array<ref GasTankRefillMapTanks>;

    private static ref GasTankRefillConfig m_Instance;
    private static bool m_IsSynced = false;
    //! tanks active on the current map (normalized), built from the lists above or received from the server
    private static ref array<string> m_ActiveTanks;
    //! server only: refills started at each tank since the server start
    private static ref map<string, int> m_TankUses;

    static const int CURRENT_VERSION = 4;
    static const int RPC_SYNC_CONFIG = 574893201;
    static const float TARGET_RAY_DISTANCE = 5.0;
    static const string CONFIG_FOLDER = "$profile:GasTankRefillMod";
    static const string CONFIG_FILE = "$profile:GasTankRefillMod/config.json";

    static GasTankRefillConfig CreateDefault()
    {
        GasTankRefillConfig cfg = new GasTankRefillConfig();
        AddDefaultTanks(cfg.AllowedGasTanks);
        AddDefaultMapTanks(cfg);
        return cfg;
    }

    //! the two tanks of the original mod (Chernarus)
    protected static void AddDefaultTanks(array<string> list)
    {
        list.Insert("tanks\\tank_small_white.p3d");
        list.Insert("tanks\\tank_small_gas.p3d");
    }

    //! maps without those tanks get their own gas tanks (objects are created inside their owner)
    protected static void AddDefaultMapTanks(GasTankRefillConfig cfg)
    {
        GasTankRefillMapTanks entry;

        entry = AddMap(cfg, "enoch");
        entry.GasTanks.Insert("tanks\\tank_small_rusty.p3d");
    }

    protected static GasTankRefillMapTanks AddMap(GasTankRefillConfig cfg, string world)
    {
        GasTankRefillMapTanks entry = new GasTankRefillMapTanks();
        cfg.MapGasTanks.Insert(entry);
        entry.MapName = world;
        return entry;
    }

    static GasTankRefillConfig GetInstance()
    {
        if (!m_Instance)
            m_Instance = CreateDefault();
        return m_Instance;
    }

    static bool IsSynced()
    {
        return m_IsSynced;
    }

    static void Load()
    {
        if (!GetGame().IsServer())
            return;

        if (!FileExist(CONFIG_FOLDER))
        {
            MakeDirectory(CONFIG_FOLDER);
            Print("[GasTankRefillMod] Created config folder: " + CONFIG_FOLDER);
        }

        GasTankRefillConfig cfg;
        if (FileExist(CONFIG_FILE))
        {
            cfg = new GasTankRefillConfig();
            string error;
            if (JsonFileLoader<GasTankRefillConfig>.LoadFile(CONFIG_FILE, cfg, error))
            {
                Print("[GasTankRefillMod] Config loaded from: " + CONFIG_FILE);
                if (Upgrade(cfg))
                    SaveConfig(cfg);
            }
            else
            {
                Print("[GasTankRefillMod] ERROR: " + error);
                Print("[GasTankRefillMod] Using the default settings, the file was not changed.");
                cfg = CreateDefault();
            }
        }
        else
        {
            cfg = CreateDefault();
            SaveConfig(cfg);
            Print("[GasTankRefillMod] Default config created at: " + CONFIG_FILE);
        }

        Validate(cfg);
        m_Instance = cfg;
        BuildActiveTanks(cfg);
        m_IsSynced = true;
        PrintConfig();
    }

    //! fills what an older or hand-edited file is missing, returns true when the file should be rewritten
    protected static bool Upgrade(GasTankRefillConfig cfg)
    {
        bool changed = false;

        if (!cfg.AllowedGasTanks)
        {
            cfg.AllowedGasTanks = new array<string>;
            changed = true;
        }

        if (!cfg.MapGasTanks)
        {
            cfg.MapGasTanks = new array<ref GasTankRefillMapTanks>;
            changed = true;
        }

        if (cfg.ConfigVersion < 3)
        {
            // version 3 added the per-map tanks; the tank list of the server stays as it was
            if (cfg.MapGasTanks.Count() == 0)
                AddDefaultMapTanks(cfg);
        }

        if (cfg.ConfigVersion < CURRENT_VERSION)
        {
            // version 4 added the refill settings, a file without them gets the default values
            Print("[GasTankRefillMod] Config upgraded from version " + cfg.ConfigVersion.ToString() + " to " + CURRENT_VERSION.ToString());
            cfg.ConfigVersion = CURRENT_VERSION;
            changed = true;
        }

        foreach (GasTankRefillMapTanks entry : cfg.MapGasTanks)
        {
            if (entry && !entry.GasTanks)
            {
                entry.GasTanks = new array<string>;
                changed = true;
            }
        }

        return changed;
    }

    //! keeps the settings in a safe range (the file itself is not changed)
    protected static void Validate(GasTankRefillConfig cfg)
    {
        cfg.TankFillPercent = ClampSetting("TankFillPercent", cfg.TankFillPercent, 1, 100);
        cfg.TankFillTimeMultiplier = ClampSetting("TankFillTimeMultiplier", cfg.TankFillTimeMultiplier, 0.1, 10);
        cfg.TankReachDistance = ClampSetting("TankReachDistance", cfg.TankReachDistance, 0.5, 3.5);
        cfg.LighterFillTimeMultiplier = ClampSetting("LighterFillTimeMultiplier", cfg.LighterFillTimeMultiplier, 0.1, 10);
        cfg.TankRefillsPerRestart = ClampSetting("TankRefillsPerRestart", cfg.TankRefillsPerRestart, 0, 100000);
        cfg.LighterRefillsPerCan = ClampSetting("LighterRefillsPerCan", cfg.LighterRefillsPerCan, 1, 100);
    }

    protected static float ClampSetting(string name, float value, float min, float max)
    {
        float clamped = Math.Clamp(value, min, max);
        if (clamped != value)
            Print("[GasTankRefillMod] WARNING: " + name + " = " + value.ToString() + " is out of range, using " + clamped.ToString());
        return clamped;
    }

    static void Save()
    {
        if (!GetGame().IsServer())
            return;

        SaveConfig(GetInstance());
    }

    protected static void SaveConfig(GasTankRefillConfig cfg)
    {
        if (!FileExist(CONFIG_FOLDER))
            MakeDirectory(CONFIG_FOLDER);

        string error;
        if (JsonFileLoader<GasTankRefillConfig>.SaveFile(CONFIG_FILE, cfg, error))
            Print("[GasTankRefillMod] Config saved to: " + CONFIG_FILE);
        else
            Print("[GasTankRefillMod] ERROR: " + error);
    }

    protected static string GetMapName()
    {
        string world = GetGame().GetWorldName();
        world.ToLower();
        return world;
    }

    //! AllowedGasTanks + the MapGasTanks entry of the current map
    protected static void BuildActiveTanks(GasTankRefillConfig cfg)
    {
        m_ActiveTanks = new array<string>;

        if (cfg.AllowedGasTanks)
        {
            foreach (string tank : cfg.AllowedGasTanks)
                AddActiveTank(tank);
        }

        if (!cfg.MapGasTanks)
            return;

        string world = GetMapName();
        foreach (GasTankRefillMapTanks entry : cfg.MapGasTanks)
        {
            if (!entry || !entry.GasTanks)
                continue;

            string entryWorld = entry.MapName;
            entryWorld.ToLower();
            if (entryWorld != world)
                continue;

            foreach (string mapTank : entry.GasTanks)
                AddActiveTank(mapTank);
        }
    }

    protected static void AddActiveTank(string tank)
    {
        string normalized = NormalizeIdentifier(tank);
        normalized.TrimInPlace();
        if (normalized != "" && m_ActiveTanks.Find(normalized) == -1)
            m_ActiveTanks.Insert(normalized);
    }

    static array<string> GetActiveTanks()
    {
        if (!m_ActiveTanks)
            BuildActiveTanks(GetInstance());
        return m_ActiveTanks;
    }

    static void PrintConfig()
    {
        GasTankRefillConfig cfg = GetInstance();
        array<string> active = GetActiveTanks();
        Print("[GasTankRefillMod] ========== CONFIG ==========");
        Print("[GasTankRefillMod] ModEnabled: " + cfg.ModEnabled.ToString());
        Print("[GasTankRefillMod] DebugLogging: " + cfg.DebugLogging.ToString());
        Print("[GasTankRefillMod] ConfigVersion: " + cfg.ConfigVersion.ToString());
        Print("[GasTankRefillMod] TankFillPercent: " + cfg.TankFillPercent.ToString());
        Print("[GasTankRefillMod] TankFillTimeMultiplier: " + cfg.TankFillTimeMultiplier.ToString());
        Print("[GasTankRefillMod] TankReachDistance: " + cfg.TankReachDistance.ToString() + " m");
        Print("[GasTankRefillMod] TankRefillsPerRestart: " + cfg.TankRefillsPerRestart.ToString() + " (0 = unlimited)");
        Print("[GasTankRefillMod] LighterRefillsPerCan: " + cfg.LighterRefillsPerCan.ToString());
        Print("[GasTankRefillMod] LighterFillTimeMultiplier: " + cfg.LighterFillTimeMultiplier.ToString());
        Print("[GasTankRefillMod] Gas tanks on " + GetMapName() + " (" + active.Count().ToString() + "):");

        for (int i = 0; i < active.Count(); i++)
            Print("[GasTankRefillMod]   - " + active[i]);

        Print("[GasTankRefillMod] ============================");
    }

    static void SendConfigToPlayer(PlayerBase player)
    {
        if (!GetGame().IsServer() || !player || !player.GetIdentity())
            return;

        GasTankRefillConfig cfg = GetInstance();
        array<string> active = GetActiveTanks();
        Param6<bool, float, float, float, float, ref array<string>> params = new Param6<bool, float, float, float, float, ref array<string>>(cfg.ModEnabled, cfg.TankFillPercent, cfg.TankFillTimeMultiplier, cfg.LighterFillTimeMultiplier, cfg.TankReachDistance, active);

        GetGame().RPCSingleParam(player, RPC_SYNC_CONFIG, params, true, player.GetIdentity());
    }

    static void SendConfigToAllPlayers()
    {
        if (!GetGame().IsServer())
            return;

        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);

        foreach (Man man : players)
        {
            PlayerBase player = PlayerBase.Cast(man);
            if (player && player.GetIdentity())
                SendConfigToPlayer(player);
        }
    }

    static void OnReceiveConfig(ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;

        Param6<bool, float, float, float, float, ref array<string>> params = new Param6<bool, float, float, float, float, ref array<string>>(false, 100, 1, 1, 2, null);

        if (!ctx.Read(params))
        {
            Print("[GasTankRefillMod] ERROR: Failed to read config from server!");
            return;
        }

        // the client needs what decides when the action is offered and how long it takes
        GasTankRefillConfig cfg = GetInstance();
        cfg.ModEnabled = params.param1;
        cfg.TankFillPercent = params.param2;
        cfg.TankFillTimeMultiplier = params.param3;
        cfg.LighterFillTimeMultiplier = params.param4;
        cfg.TankReachDistance = params.param5;

        // the server sends the tanks that are active on this map
        m_ActiveTanks = new array<string>;
        if (params.param6)
        {
            for (int i = 0; i < params.param6.Count(); i++)
                AddActiveTank(params.param6[i]);
        }

        m_IsSynced = true;
        PrintConfig();
    }

    // ------------------------------------------------------------------ refill amounts and times

    //! how full a canister gets at a tank (0.01 - 1)
    static float GetTankFillFraction()
    {
        return GetInstance().TankFillPercent / 100.0;
    }

    //! seconds to fill this canister at a tank (6 / 8 / 10 / 14 s from empty to full)
    static float GetTankFillTime(ItemBase canister, float currentEnergy, float maxEnergy)
    {
        float baseTimeFull = 8.0;
        if (canister)
        {
            if (canister.IsKindOf("GasLighterRefillCan"))
                baseTimeFull = 6.0;
            else if (canister.IsKindOf("SmallGasCanister"))
                baseTimeFull = 8.0;
            else if (canister.IsKindOf("MediumGasCanister"))
                baseTimeFull = 10.0;
            else if (canister.IsKindOf("LargeGasCanister"))
                baseTimeFull = 14.0;
        }

        float timeToFill = baseTimeFull;
        float missingEnergy = maxEnergy * GetTankFillFraction() - currentEnergy;
        if (maxEnergy > 0 && missingEnergy > 0)
            timeToFill = baseTimeFull * (missingEnergy / maxEnergy);

        timeToFill = timeToFill * GetInstance().TankFillTimeMultiplier;
        if (timeToFill < 1.0)
            timeToFill = 1.0;

        return timeToFill;
    }

    //! seconds to refill this lighter (4 s from empty to full)
    static float GetLighterFillTime(ItemBase lighter)
    {
        float baseTimeFull = 4.0;
        float timeToFill = baseTimeFull;

        if (lighter)
        {
            float max = lighter.GetQuantityMax();
            float missing = max - lighter.GetQuantity();
            if (max > 0 && missing > 0)
                timeToFill = baseTimeFull * (missing / max);
        }

        timeToFill = timeToFill * GetInstance().LighterFillTimeMultiplier;
        if (timeToFill <= 0.5)
            timeToFill = 0.5;

        return timeToFill;
    }

    // ------------------------------------------------------------------ tank limit per restart (server)

    protected static string GetTankKey(Object tank)
    {
        vector pos = tank.GetPosition();
        return Math.Round(pos[0] * 10).ToString() + ":" + Math.Round(pos[2] * 10).ToString();
    }

    static bool IsTankEmpty(Object tank)
    {
        int limit = GetInstance().TankRefillsPerRestart;
        if (limit <= 0 || !tank || !m_TankUses)
            return false;

        int used;
        if (!m_TankUses.Find(GetTankKey(tank), used))
            return false;

        return used >= limit;
    }

    //! every started refill uses the tank once
    static void UseTank(Object tank)
    {
        if (GetInstance().TankRefillsPerRestart <= 0 || !tank)
            return;

        if (!m_TankUses)
            m_TankUses = new map<string, int>;

        string key = GetTankKey(tank);
        int used;
        m_TankUses.Find(key, used);
        used++;
        m_TankUses.Set(key, used);

        if (GetInstance().DebugLogging)
        {
            string msg = "[GasTankRefillMod] tank at " + tank.GetPosition().ToString();
            Print(msg + " used " + used.ToString() + " of " + GetInstance().TankRefillsPerRestart.ToString());
        }
    }

    //! "The gas tank is empty" for the player (at most every 3 s; never without an identity - that would go to everyone)
    static void NotifyTankEmpty(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;

        int now = GetGame().GetTime();
        if (player.m_GTR_LastEmptyNotice > 0 && now - player.m_GTR_LastEmptyNotice < 3000)
            return;

        player.m_GTR_LastEmptyNotice = now;
        NotificationSystem.SendNotificationToPlayerExtended(player, 4, "#STR_GTR_TankEmptyTitle", "#STR_GTR_TankEmptyText");
    }

    // ------------------------------------------------------------------ which map objects are refill tanks

    protected static string NormalizeIdentifier(string value)
    {
        value.ToLower();
        value.Replace("/", "\\");
        return value;
    }

    protected static bool MatchesAllowedIdentifier(string candidate)
    {
        if (candidate == "")
            return false;

        array<string> active = GetActiveTanks();
        if (active.Count() == 0)
            return false;

        candidate = NormalizeIdentifier(candidate);

        foreach (string allowedTank : active)
        {
            if (candidate.Contains(allowedTank))
                return true;
        }

        return false;
    }

    static bool IsAllowedGasTank(Object obj)
    {
        if (!obj)
            return false;

        GasTankRefillConfig cfg = GetInstance();
        if (!cfg.ModEnabled)
            return false;

        // Do not hide the action while waiting for the initial RPC. The local
        // default list is safe to use and the server still validates the action.
        if (MatchesAllowedIdentifier(obj.GetShapeName()))
            return true;

        // Also permits server owners to put a class name in AllowedGasTanks.
        if (MatchesAllowedIdentifier(obj.GetType()))
            return true;

        return false;
    }

    static bool IsAllowedGasTankTarget(PlayerBase player, ActionTarget target)
    {
        return FindTargetTank(player, target) != null;
    }

    //! the refill tank the player aims at and can reach, null when there is none
    static Object FindTargetTank(PlayerBase player, ActionTarget target)
    {
        if (!player || !target)
            return null;

        if (!GetInstance().ModEnabled)
            return null;

        // DayZ 1.29.163451 may expose a proxy's parent instead of the object
        // that was returned before the ActionTargets rewrite.
        // The reach check also stops requests for a tank far away.
        Object targetObject = target.GetObject();
        if (IsAllowedGasTank(targetObject) && IsWithinReach(player, targetObject))
            return targetObject;

        Object targetParent = target.GetParent();
        if (IsAllowedGasTank(targetParent) && IsWithinReach(player, targetParent))
            return targetParent;

        // Compatibility fallback for proxies filtered by the new
        // "res.obj.GetID() < 1" rule in vanilla ActionTargets.
        return RaycastAllowedGasTank(player, target);
    }

    protected static Object RaycastAllowedGasTank(PlayerBase player, ActionTarget target)
    {
        if (!player)
            return null;

        vector rayStart;
        vector rayEnd;
        vector playerHeadPos;
        MiscGameplayFunctions.GetHeadBonePos(player, playerHeadPos);

        if (GetGame().IsClient())
        {
            rayStart = GetGame().GetCurrentCameraPosition();
            rayEnd = rayStart + GetGame().GetCurrentCameraDirection() * TARGET_RAY_DISTANCE;
        }
        else
        {
            rayStart = playerHeadPos;

            vector cursorHitPos = vector.Zero;
            if (target)
                cursorHitPos = target.GetCursorHitPos();

            vector rayDirection;
            if (cursorHitPos != vector.Zero)
                rayDirection = cursorHitPos - rayStart;
            else
                rayDirection = player.GetDirection();

            if (rayDirection == vector.Zero)
                return null;

            rayDirection.Normalize();
            rayEnd = rayStart + rayDirection * TARGET_RAY_DISTANCE;
        }

        RaycastRVParams rayInput = new RaycastRVParams(rayStart, rayEnd, player);
        rayInput.flags = CollisionFlags.ALLOBJECTS;

        array<ref RaycastRVResult> results = new array<ref RaycastRVResult>;
        if (!DayZPhysics.RaycastRVProxy(rayInput, results))
            return null;

        // Only what the ray hits first counts: a tank behind a wall or another object is not reachable.
        // Hits within 5 cm of the nearest one belong to it (a proxy and its parent are hit at the same point).
        float nearest = float.MAX;
        foreach (RaycastRVResult hit : results)
        {
            if (IsIgnoredHit(hit))
                continue;

            float hitDistance = vector.Distance(hit.pos, rayStart);
            if (hitDistance < nearest)
                nearest = hitDistance;
        }

        foreach (RaycastRVResult result : results)
        {
            if (IsIgnoredHit(result))
                continue;

            if (vector.Distance(result.pos, rayStart) > nearest + 0.05)
                continue;

            Object found = null;
            if (IsAllowedGasTank(result.obj))
                found = result.obj;
            else if (IsAllowedGasTank(result.parent))
                found = result.parent;

            // the same reach as for a tank that is the target object
            if (found && IsWithinReach(player, found))
            {
                if (GetInstance().DebugLogging)
                {
                    Print("[GasTankRefillMod] 1.29 proxy fallback matched object=" + Object.GetDebugName(result.obj) + " parent=" + Object.GetDebugName(result.parent));
                }
                return found;
            }
        }

        return null;
    }

    //! hits vanilla ActionTargets skips as well: holograms and the gear of a player (proxies of the player)
    protected static bool IsIgnoredHit(RaycastRVResult result)
    {
        if (result.obj && !result.obj.CanBeActionTarget())
            return true;

        return result.hierLevel > 0 && result.parent && result.parent.IsMan();
    }

    //! distance from the player's feet to the collision box of the object (TankReachDistance;
    //! big tanks: Object.GetPosition() is their centre, so the box is used)
    protected static bool IsWithinReach(PlayerBase player, Object obj)
    {
        vector minMax[2];
        if (!obj.GetCollisionBox(minMax))
            obj.ClippingInfo(minMax);

        vector playerInModel = obj.WorldToModel(player.GetPosition());
        vector lo = minMax[0];
        vector hi = minMax[1];
        vector closest = Vector(Math.Clamp(playerInModel[0], lo[0], hi[0]), Math.Clamp(playerInModel[1], lo[1], hi[1]), Math.Clamp(playerInModel[2], lo[2], hi[2]));

        return vector.Distance(playerInModel, closest) <= GetInstance().TankReachDistance;
    }
};

// CCTObject measures distance to Object.GetPosition(). After the 1.29 target
// rewrite that position can belong to a proxy parent, not to the point aimed at.
// This lightweight condition lets ActionCondition perform the authoritative
// object/parent/proxy check and its own distance-limited raycast.
class CCTGasTankRefill : CCTBase
{
    override bool Can(PlayerBase player, ActionTarget target)
    {
        return player && target;
    }
};
