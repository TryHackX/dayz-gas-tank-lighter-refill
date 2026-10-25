class ActionRefillLighterFromGasCB : ActionContinuousBaseCB
{
    override void CreateActionComponent()
    {
        ItemBase lighter = null;

        if (m_ActionData && m_ActionData.m_MainItem && m_ActionData.m_Target)
        {
            ItemBase item1 = ItemBase.Cast(m_ActionData.m_MainItem);
            ItemBase item2 = ItemBase.Cast(m_ActionData.m_Target.GetObject());

            if (item1 && item1.IsKindOf("PetrolLighter")) lighter = item1;
            else if (item2 && item2.IsKindOf("PetrolLighter")) lighter = item2;
        }

        m_ActionData.m_ActionComponent = new CAContinuousTime(GasTankRefillConfig.GetLighterFillTime(lighter));
    }
};

// Actions are shared by all players, so the per-player progress lives in the action data.
class ActionRefillLighterFromGasData : ActionData
{
    float m_LighterStart;
    float m_LighterMax;
    float m_GasStart;
    float m_TransferRatio;
};

class ActionRefillLighterFromGas : ActionContinuousBase
{
    void ActionRefillLighterFromGas()
    {
        m_CallbackClass = ActionRefillLighterFromGasCB;
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_CRAFTING;
        m_FullBody   = true;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
        m_SpecialtyWeight = UASoftSkillsWeight.PRECISE_LOW;
        m_Text            = "#STR_GTR_FillLighter";
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem   = new CCINonRuined();
        m_ConditionTarget = new CCTNonRuined(UAMaxDistances.DEFAULT);
    }

    override ActionData CreateActionData()
    {
        ActionRefillLighterFromGasData actionData = new ActionRefillLighterFromGasData();
        return actionData;
    }

    bool SetupVariables(ItemBase item1, ItemBase item2, out ItemBase gasCan, out ItemBase lighter)
    {
        if (!item1 || !item2) return false;

        if (item1.IsKindOf("GasLighterRefillCan") && item2.IsKindOf("PetrolLighter"))
        {
            gasCan = item1;
            lighter = item2;
            return true;
        }
        else if (item1.IsKindOf("PetrolLighter") && item2.IsKindOf("GasLighterRefillCan"))
        {
            lighter = item1;
            gasCan = item2;
            return true;
        }
        return false;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!player || !item || !target) return false;

        ItemBase targetItem = ItemBase.Cast(target.GetObject());
        ItemBase gasCan = null;
        ItemBase lighter = null;

        if (!SetupVariables(item, targetItem, gasCan, lighter)) return false;

        if (lighter.GetQuantity() >= lighter.GetQuantityMax()) return false;
        if (gasCan.GetQuantity() <= 0) return false;

        return true;
    }

    override void OnStartServer(ActionData action_data)
    {
        super.OnStartServer(action_data);

        ActionRefillLighterFromGasData data = ActionRefillLighterFromGasData.Cast(action_data);
        if (!data) return;

        data.m_LighterStart = 0;
        data.m_LighterMax   = 0;
        data.m_GasStart     = 0;
        data.m_TransferRatio = 1.0;

        if (!data.m_MainItem || !data.m_Target) return;

        ItemBase itemInHand = data.m_MainItem;
        ItemBase itemTarget = ItemBase.Cast(data.m_Target.GetObject());
        ItemBase gasCan = null;
        ItemBase lighter = null;

        if (SetupVariables(itemInHand, itemTarget, gasCan, lighter))
        {
            data.m_LighterStart = lighter.GetQuantity();
            data.m_LighterMax   = lighter.GetQuantityMax();
            data.m_GasStart     = gasCan.GetQuantity();

            // one Lighter gas can refills an empty lighter LighterRefillsPerCan times
            float canisterMax = gasCan.GetQuantityMax();
            float gasCostPerRefill = canisterMax / GasTankRefillConfig.GetInstance().LighterRefillsPerCan;

            if (data.m_LighterMax > 0)
                data.m_TransferRatio = gasCostPerRefill / data.m_LighterMax;
            else
                data.m_TransferRatio = 1.0;
        }
    }

    override void OnUpdateServer(ActionData action_data)
    {
        super.OnUpdateServer(action_data);

        ActionRefillLighterFromGasData data = ActionRefillLighterFromGasData.Cast(action_data);
        if (!data || !data.m_MainItem || !data.m_Target) return;

        ItemBase itemInHand = data.m_MainItem;
        ItemBase itemTarget = ItemBase.Cast(data.m_Target.GetObject());
        ItemBase gasCan = null;
        ItemBase lighter = null;

        if (SetupVariables(itemInHand, itemTarget, gasCan, lighter))
        {
            float progress = GetProgress(data);
            if (progress < 0) return;
            if (progress > 1.0) progress = 1.0;

            float lighterMissing = data.m_LighterMax - data.m_LighterStart;
            float amountToAddToLighter = lighterMissing * progress;
            float amountToTakeFromGas = amountToAddToLighter * data.m_TransferRatio;

            if (amountToTakeFromGas > data.m_GasStart)
            {
                amountToTakeFromGas = data.m_GasStart;
                amountToAddToLighter = amountToTakeFromGas / data.m_TransferRatio;
            }

            float newLighter = data.m_LighterStart + amountToAddToLighter;
            float newGas     = data.m_GasStart     - amountToTakeFromGas;

            if (newLighter > data.m_LighterMax) newLighter = data.m_LighterMax;
            if (newGas < 0) newGas = 0;

            lighter.SetQuantity(newLighter);
            gasCan.SetQuantity(newGas);
        }
    }

    override void OnFinishProgressServer(ActionData action_data)
    {
        super.OnFinishProgressServer(action_data);

        ActionRefillLighterFromGasData data = ActionRefillLighterFromGasData.Cast(action_data);
        if (!data || !data.m_MainItem || !data.m_Target) return;

        ItemBase itemInHand = data.m_MainItem;
        ItemBase itemTarget = ItemBase.Cast(data.m_Target.GetObject());
        ItemBase gasCan = null;
        ItemBase lighter = null;

        if (SetupVariables(itemInHand, itemTarget, gasCan, lighter))
        {
            float neededForLighter = data.m_LighterMax - data.m_LighterStart;
            float costForGas       = neededForLighter * data.m_TransferRatio;

            if (costForGas > data.m_GasStart)
            {
                costForGas = data.m_GasStart;
                neededForLighter = costForGas / data.m_TransferRatio;
            }

            float finalLighter = data.m_LighterStart + neededForLighter;
            float finalGas     = data.m_GasStart - costForGas;

            if (finalLighter > data.m_LighterMax) finalLighter = data.m_LighterMax;
            if (finalGas < 0.01) finalGas = 0;

            lighter.SetQuantity(finalLighter);
            gasCan.SetQuantity(finalGas);

        }
    }

    override void OnEndServer(ActionData action_data)
    {
        super.OnEndServer(action_data);

        if (!action_data || !action_data.m_MainItem || !action_data.m_Target) return;

        ItemBase itemInHand = action_data.m_MainItem;
        ItemBase itemTarget = ItemBase.Cast(action_data.m_Target.GetObject());
        ItemBase gasCan = null;
        ItemBase lighter = null;

        if (SetupVariables(itemInHand, itemTarget, gasCan, lighter))
        {
            lighter.SetSynchDirty();
            gasCan.SetSynchDirty();
        }
    }
};
