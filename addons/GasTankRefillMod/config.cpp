class CfgPatches
{
    class GasTankRefillMod
    {
        units[] = {"GasLighterRefillCan"};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Consumables"};
    };
};

class CfgMods
{
    class GasTankRefillMod
    {
        dir = "GasTankRefillMod";
        picture = "";
        action = "";
        hideName = 1;
        hidePicture = 1;
        name = "Gas Tank Refill Mod";
        credits = "";
        author = "TryHackX";
        authorID = "76561199846115789";
        version = "1.6.0";
        extra = 0;
        type = "mod";
        dependencies[] = {"World", "Mission"};

        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"GasTankRefillMod/Scripts/4_World"};
            };

            class missionScriptModule
            {
                value = "";
                files[] = {"GasTankRefillMod/Scripts/5_Mission"};
            };
        };
    };
};

class CfgVehicles
{
    class ButaneCanister;

    class GasLighterRefillCan : ButaneCanister
    {
        scope = 2;
        displayName = "$STR_GTR_LighterGas";
        descriptionShort = "$STR_GTR_LighterGasDesc";

        model = "\dz\gear\consumables\ButaneCanister.p3d";

        varQuantityInit = 0;
        varQuantityMin = 0;
        varQuantityMax = 300;
        varQuantityDestroyOnMin = 0;
        quantityBar = 1;

        hiddenSelections[] = {"zbytek"};
        hiddenSelectionsTextures[] = {"\GasTankRefillMod\data\gas_lighter_refill_co.paa"};

    };
};
