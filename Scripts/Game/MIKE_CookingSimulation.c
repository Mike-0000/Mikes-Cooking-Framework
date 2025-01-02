class ProcessCookingSimulation
{
    // Cooking-specific fields
	[RplProp()]
    private bool   m_bActive;
	[RplProp()]
    private bool   m_bHasProcess;
	[RplProp()]
    private bool   m_bProcessFinalized;
    
	
    private float  m_fRawRisk;
    private float  m_fBurnRisk;
    private float  m_overCookedRisk;
    
    private string m_sItemName;
    private float  m_fOptimalHeatMin;
    private float  m_fOptimalHeatMax;
    private string recipeName;
    private float  m_fCurrentTime;
	[RplProp()]
    private float  m_fHeatSum;
    private float  m_fHeatSamples;
	bool  m_bisDestroyed;
	IEntity newOwnerEntity;
	int BoilIndex;
    
	SCR_DestructionMultiPhaseComponent destructionComp;
	SCR_ExplosiveChargeComponent explosiveComp;
    private int    m_iQualityScore;
    private string m_sOutputItem;
	//SignalsManagerComponent GameSignal;
    // Reference to the stove
    private ref StoveSimulation m_StoveSim;
    SCR_UniversalInventoryStorageComponent StorageComp;
    void ProcessCookingSimulation(StoveSimulation stoveSim, SCR_UniversalInventoryStorageComponent StorageComp1)
    {
        // Store a reference to the stove
        m_StoveSim = stoveSim;
		StorageComp = StorageComp1;
        m_bActive           = false;
        m_bHasProcess       = false;
        m_bProcessFinalized = false;
        m_sItemName         = "";
        m_fOptimalHeatMin   = 0.0;
        m_fOptimalHeatMax   = 0.0;
		recipeName = "";
        m_fCurrentTime      = 0.0;
        m_fHeatSum          = 0.0;
        m_fHeatSamples      = 0.0;
        m_overCookedRisk    = 0.0;
		BoilIndex = 0;

        m_iQualityScore     = 0;
        m_sOutputItem       = "";
        m_fRawRisk          = 1.0;
        m_fBurnRisk         = 0.0;
		m_bisDestroyed = false;
    }
    void StartProcess(string itemName, float optimalHeatMin, float optimalHeatMax, IEntity ownerEntity, string recipeName1)
    {
		
        if (m_bActive)
        {
            Print("[CookingSimulation] Already active; cannot start new process.", LogLevel.WARNING);
            return;
        }

//        BoilIndex = GameSignal.AddOrFindMPSignal("StoveTemp", 2, 0.1, 70);
//		GameSignal.AddOrFindMPSignal("optimalHeatMin", 2, 0.1, optimalHeatMin);
//		GameSignal.AddOrFindMPSignal("optimalHeatMax", 2, 0.1, optimalHeatMax);

		newOwnerEntity = ownerEntity;
		destructionComp = SCR_DestructionMultiPhaseComponent.Cast(ownerEntity.FindComponent(SCR_DestructionMultiPhaseComponent));
        explosiveComp = SCR_ExplosiveChargeComponent.Cast(ownerEntity.FindComponent(SCR_ExplosiveChargeComponent));
		m_sItemName       = itemName;
        m_fOptimalHeatMin = optimalHeatMin;
        m_fOptimalHeatMax = optimalHeatMax;
        m_fCurrentTime    = 0.0;
        m_fHeatSum        = 0.0;
        m_fHeatSamples    = 0.0;
		recipeName = recipeName1;
        m_fRawRisk   = 1.0;
        m_fBurnRisk  = 0.0;
        m_overCookedRisk = 0.0;
        m_bActive           = true;
        m_bHasProcess       = true;
        m_bProcessFinalized = false;
        m_iQualityScore     = 0;
        m_sOutputItem       = "";

        Print("[CookingSimulation] Process started for item: " + itemName, LogLevel.NORMAL);
    }
	int hitZoneDamageToDo = 0;
    void Update(float deltaTime)
    {
		if(m_bisDestroyed){
			
			return;
		}
        if (!m_bActive || !m_bHasProcess || !m_StoveSim)
            return;
        
        // 1. Increase elapsed time
        m_fCurrentTime += deltaTime;
        
        // 2. Pull the stove’s current heat
        float stoveHeat = 0.0;
        if (m_StoveSim){
            stoveHeat = m_StoveSim.GetCurrentStoveHeat();
			m_StoveSim.ResetIdleTimer();
		}
		 Print("Stove Heat: " + stoveHeat, LogLevel.NORMAL);

        // 3. Record it for averaging (unchanged)
        m_fHeatSum     += stoveHeat * deltaTime;
        m_fHeatSamples += deltaTime;

        // 4. Raw and Burn Risk (exact math as you had before)
        if (m_fRawRisk > 0.0)
        {
            if (stoveHeat >= m_fOptimalHeatMin && stoveHeat <= m_fOptimalHeatMax)
            {
                m_fRawRisk -= deltaTime * 0.02;
                if (m_fRawRisk < 0.0) 
                    m_fRawRisk = 0.0;
            }
            else if (stoveHeat > m_fOptimalHeatMax)
            {
                // both decaying raw risk and building burn risk
                m_fRawRisk   -= deltaTime * 0.06;
                m_fBurnRisk  += deltaTime * 0.0005 * (stoveHeat - m_fOptimalHeatMax);
            }
        }
        else if (m_fBurnRisk < 1.0)
        {
            if (stoveHeat > m_fOptimalHeatMax)
            {
                m_fBurnRisk += deltaTime * 0.1;
                if (m_fBurnRisk > 1.0) 
                    m_fBurnRisk = 1.0;
            }
        }
		
		// METH LAB DESTRUCTION LOGIC
		
		if (stoveHeat > m_fOptimalHeatMax+40 && recipeName == "Meth"){
			hitZoneDamageToDo += 200;
			if(destructionComp.GetHealth() <= 500 && !m_bisDestroyed){
//				Print("GOING FOR EXPLOSION", LogLevel.WARNING);
				explosiveComp.SetFuzeTime(3,false);
				explosiveComp.ArmWithTimedFuze(false);
				m_bisDestroyed = true;
				m_bActive = false;
				GetGame().GetCallqueue().CallLater(destructionComp.InitDestruction, 2999, false);
				GetGame().GetCallqueue().CallLater(destructionComp.SetHitZoneHealth, 2999, false, 0, false); 
				GetGame().GetCallqueue().CallLater(m_StoveSim.StopStove, 2999, false); 



			}
			destructionComp.SetHitZoneDamage(hitZoneDamageToDo);
		}
		if(m_fRawRisk < 0.0){
			m_fRawRisk = 0.0;
		}
        // Overcooked risk when raw risk is zero but heat is above minimal
        if (m_fRawRisk == 0.0 && stoveHeat > m_fOptimalHeatMin)
        {
            m_overCookedRisk += deltaTime * 0.01;
        }
//		if (BoilIndex == 0)
//			BoilIndex = GameSignal.AddOrFindMPSignal("StoveHeat", 2, 0.1, stoveHeat);
		
//		GameSignal.SetSignalValue(BoilIndex, stoveHeat);
        // Debugging logs
        Print("[CookingSimulation] stoveHeat=" + stoveHeat
            + " | RawRisk=" + m_fRawRisk
            + " | BurnRisk=" + m_fBurnRisk
            + " | OverCookRisk=" + m_overCookedRisk
			+ " | HealthPoints=" + destructionComp.GetHealth()
            + " | Time=" + m_fCurrentTime, LogLevel.NORMAL);
    }

    void FinalizeProcess()
    {
        if (!m_bActive || !m_bHasProcess)
        {
            Print("[CookingSimulation] No active process to finalize.", LogLevel.WARNING);
            return;
        }

        m_bActive = false;
        m_bProcessFinalized = true;

        // Use your existing final quality math
        int quality = 100;
        
        if (m_fRawRisk > 0.05)
            quality -= (int)(m_fRawRisk * 200);

        if (m_fBurnRisk > 0.05)
            quality -= (int)(m_fBurnRisk * 200);

        if (m_overCookedRisk > 0.1)
            quality -= (int)(m_overCookedRisk * 250);

        if (quality < 0)
            quality = 0;

        // Perfect condition
        if (m_fRawRisk == 0.0 && m_fBurnRisk == 0.0 && m_overCookedRisk < 0.05)
        {
            quality = 100;
            Print("[CookingSimulation] Perfect Cooking Achieved!", LogLevel.NORMAL);
        }
		
		

        m_iQualityScore = quality;
    }
	

    bool IsProcessActive()
    {
        return m_bActive;
    }

    bool IsProcessFinalized()
    {
        return m_bProcessFinalized;
    }

    float GetCurrentTime()
    {
        return m_fCurrentTime;
    }

    string GetOutcomeItem()
    {
        return m_sOutputItem;
    }

    int GetQualityScore()
    {
        return m_iQualityScore;
    }
}
