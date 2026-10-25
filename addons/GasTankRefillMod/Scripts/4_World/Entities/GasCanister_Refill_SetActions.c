modded class LargeGasCanister
{
    override void SetActions()
    {
        super.SetActions();
        AddAction(ActionRefillGasCanister);
    }
};

modded class MediumGasCanister
{
    override void SetActions()
    {
        super.SetActions();
        AddAction(ActionRefillGasCanister);
    }
};

modded class SmallGasCanister
{
    override void SetActions()
    {
        super.SetActions();
        AddAction(ActionRefillGasCanister);
    }
};

modded class PetrolLighter
{
    override void SetActions()
    {
        super.SetActions();
        AddAction(ActionRefillLighterFromGas);
    }
};
