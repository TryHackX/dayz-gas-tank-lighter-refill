class GasLighterRefillCan : ButaneCanister
{
    override void SetActions()
    {
        super.SetActions();
        AddAction(ActionRefillGasCanister);
        AddAction(ActionRefillLighterFromGas);
    }

    override bool CanSwitchDuringAttach(EntityAI parent)
    {
        return true;
    }

    override string GetDestructionBehaviour()
    {
        return "DestructionEffectGasCanister";
    }

    override bool IsDestructionBehaviour()
    {
        return GetQuantity() > 0;
    }
};
