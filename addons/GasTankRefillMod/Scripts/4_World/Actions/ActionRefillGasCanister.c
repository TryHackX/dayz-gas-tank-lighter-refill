class ActionRefillGasCanisterCB : ActionContinuousBaseCB
{
    override void CreateActionComponent()
    {
        ItemBase canister;
        if (m_ActionData)
            canister = ItemBase.Cast(m_ActionData.m_MainItem);

        float currentEnergy = 0;
        float maxEnergy = 0;
        if (canister)
        {
            currentEnergy = GetCurrentEnergy(canister);
            maxEnergy = GetMaxEnergy(canister);
        }

        m_ActionData.m_ActionComponent = new CAContinuousTime(GasTankRefillConfig.GetTankFillTime(canister, currentEnergy, maxEnergy));
    }

    protected float GetCurrentEnergy(ItemBase item)
    {
        if (item && item.HasEnergyManager())
            return item.GetCompEM().GetEnergy();

        if (item)
            return item.GetQuantity();

        return 0;
    }

    protected float GetMaxEnergy(ItemBase item)
    {
        if (item && item.HasEnergyManager())
            return item.GetCompEM().GetEnergyMax();

        if (item)
            return item.GetQuantityMax();

        return 0;
    }
};

// Actions are shared by all players, so the per-player progress lives in the action data.
class ActionRefillGasCanisterData : ActionData
{
    float m_StartEnergy;
    float m_TargetEnergy;
};

class ActionRefillGasCanister : ActionContinuousBase
{
    void ActionRefillGasCanister()
    {
        m_CallbackClass = ActionRefillGasCanisterCB;
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_FILLBOTTLEWELL;
        m_FullBody = true;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
        m_SpecialtyWeight = UASoftSkillsWeight.PRECISE_LOW;
        m_Text = "#STR_GTR_FillContainer";
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem = new CCINonRuined();
        m_ConditionTarget = new CCTGasTankRefill();
    }

    override ActionData CreateActionData()
    {
        ActionRefillGasCanisterData actionData = new ActionRefillGasCanisterData();
        return actionData;
    }

    protected float GetCurrentEnergy(ItemBase item)
    {
        if (item && item.HasEnergyManager())
            return item.GetCompEM().GetEnergy();

        if (item)
            return item.GetQuantity();

        return 0;
    }

    protected float GetMaxEnergy(ItemBase item)
    {
        if (item && item.HasEnergyManager())
            return item.GetCompEM().GetEnergyMax();

        if (item)
            return item.GetQuantityMax();

        return 0;
    }

    //! energy (or quantity) the canister gets at a tank - TankFillPercent of the maximum
    protected float GetFillTarget(ItemBase item)
    {
        return GetMaxEnergy(item) * GasTankRefillConfig.GetTankFillFraction();
    }

    protected bool IsFilled(ItemBase item)
    {
        return GetCurrentEnergy(item) >= GetFillTarget(item) - 0.01;
    }

    protected void SetItemEnergy(ItemBase item, float energy)
    {
        if (!item)
            return;

        float targetEnergy = energy;
        float maxEnergyLimit = GetMaxEnergy(item);
        if (targetEnergy < 0)
            targetEnergy = 0;
        if (targetEnergy > maxEnergyLimit)
            targetEnergy = maxEnergyLimit;

        if (item.HasEnergyManager())
        {
            float maxEnergy = item.GetCompEM().GetEnergyMax();
            float quantityMax = item.GetQuantityMax();

            if (maxEnergy > 0 && quantityMax > 0)
                item.SetQuantity((targetEnergy / maxEnergy) * quantityMax);

            item.GetCompEM().SetEnergy(targetEnergy);
        }
        else
        {
            item.SetQuantity(targetEnergy);
        }

        item.SetSynchDirty();
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!player || !target || !item)
            return false;

        if (player.GetItemInHands() != item)
            return false;

        bool isGasCanister = item.IsKindOf("LargeGasCanister") || item.IsKindOf("MediumGasCanister") || item.IsKindOf("SmallGasCanister") || item.IsKindOf("GasLighterRefillCan");

        if (!isGasCanister)
            return false;

        if (GetMaxEnergy(item) <= 0 || IsFilled(item))
            return false;

        Object tank = GasTankRefillConfig.FindTargetTank(player, target);
        if (!tank)
            return false;

        // the refill limit per tank is only known by the server, it refuses and tells the player
        if (GetGame().IsServer() && GasTankRefillConfig.IsTankEmpty(tank))
        {
            GasTankRefillConfig.NotifyTankEmpty(player);
            return false;
        }

        return true;
    }

    override bool ActionConditionContinue(ActionData action_data)
    {
        if (!action_data || !action_data.m_MainItem)
            return false;

        return !IsFilled(action_data.m_MainItem);
    }

    override void OnStartServer(ActionData action_data)
    {
        super.OnStartServer(action_data);

        ActionRefillGasCanisterData data = ActionRefillGasCanisterData.Cast(action_data);
        if (!data || !data.m_MainItem)
            return;

        data.m_StartEnergy = GetCurrentEnergy(data.m_MainItem);
        data.m_TargetEnergy = GetFillTarget(data.m_MainItem);

        GasTankRefillConfig.UseTank(GasTankRefillConfig.FindTargetTank(data.m_Player, data.m_Target));
    }

    override void OnUpdateServer(ActionData action_data)
    {
        super.OnUpdateServer(action_data);

        ActionRefillGasCanisterData data = ActionRefillGasCanisterData.Cast(action_data);
        if (!data || !data.m_MainItem)
            return;

        if (data.m_TargetEnergy <= 0 || data.m_TargetEnergy <= data.m_StartEnergy)
            return;

        float progress = GetProgress(data);
        if (progress < 0)
            progress = 0;
        if (progress > 1)
            progress = 1;

        float newEnergy = data.m_StartEnergy + (data.m_TargetEnergy - data.m_StartEnergy) * progress;
        SetItemEnergy(data.m_MainItem, newEnergy);
    }

    override void OnFinishProgressServer(ActionData action_data)
    {
        super.OnFinishProgressServer(action_data);

        ActionRefillGasCanisterData data = ActionRefillGasCanisterData.Cast(action_data);
        if (!data || !data.m_MainItem)
            return;

        if (data.m_TargetEnergy > data.m_StartEnergy)
            SetItemEnergy(data.m_MainItem, data.m_TargetEnergy);

        if (GasTankRefillConfig.GetInstance().DebugLogging)
            Print("[GasTankRefillMod] " + Object.GetDebugName(data.m_Player) + " filled " + Object.GetDebugName(data.m_MainItem) + " from a gas tank");
    }

    override void OnEndServer(ActionData action_data)
    {
        super.OnEndServer(action_data);

        if (action_data && action_data.m_MainItem)
            action_data.m_MainItem.SetSynchDirty();
    }
};
