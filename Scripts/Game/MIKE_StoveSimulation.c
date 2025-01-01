class StoveSimulation
{
    // Stove-specific fields
    private bool   m_bStoveActive;
    private int    m_iStoveSetting;         // Allowed range 0–10
    private float  m_fCurrentStoveHeat = 70.0;
    private float  m_fHeatSoak = 0.0;
    private int BoilIndex;
    // Constants (same as before)
    private float  m_fThermalInertia      = 10.0;
    private float  m_fCoolingRate         = 0.02;
    private float  m_fAmbientTemperature  = 70.0;
	IEntity ownerEntity;
    private float  m_fMaxHeat             = 570.0;
    private float  m_fRate               = 0.5;    // Used if you had some approach speed logic
	SignalsManagerComponent GameSignal;
    protected AudioHandle m_AudioHandle = AudioHandle.Invalid;
	//SCR_SoundManagerEntity soundManagerEntity;
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
	private vector m_vSoundOffset;
	ref SCR_AudioSource audioSource1;
	


	SoundComponent soundComp;
    void StoveSimulation(IEntity ownerEntity1, SCR_AudioSourceConfiguration audioSource11, vector soundOffset)
    {
		m_vSoundOffset = soundOffset;
		m_AudioSourceConfiguration = audioSource11;
        m_bStoveActive = false;
        m_iStoveSetting = 0;
		ownerEntity = ownerEntity1;
		
    }
    
    void StartStove()
    {
		
		GameSignal = SignalsManagerComponent.Cast(ownerEntity.FindComponent(SignalsManagerComponent));

		BoilIndex = GameSignal.AddOrFindMPSignal("STOVE_HEAT", 0.1, 0.1, 1.0);
		soundComp = SoundComponent.Cast(ownerEntity.FindComponent(SoundComponent));
        m_bStoveActive = true;
		m_AudioHandle = soundComp.PlayStr("SOUND_BOILING");
	
    }
    
    void StopStove()
    {
        m_bStoveActive = false;
		soundComp.TerminateAll();

    }
    
    void SetStoveSetting(int setting)
    {
        if (setting < 0) setting = 0;
        if (setting > 10) setting = 10;
        m_iStoveSetting = setting;
        Print("[StoveSimulation] Stove Setting changed to: " + m_iStoveSetting, LogLevel.NORMAL);
    }
    
    void AdjustStoveSetting(float value)
    {
        if (!m_bStoveActive)
        {
            Print("[StoveSimulation] Stove is not active; cannot adjust heat.", LogLevel.WARNING);
            return;
        }

        m_iStoveSetting += (int)value;
        if (m_iStoveSetting < 0)  m_iStoveSetting = 0;
        if (m_iStoveSetting > 11) m_iStoveSetting = 11;
        
        Print("[StoveSimulation] Stove setting adjusted by " + value + ". Current Setting: " + m_iStoveSetting, LogLevel.NORMAL);
    }
    
    // Main stove temperature update (same math as before)
    void UpdateStove(float deltaTime)
    {
        if (!m_bStoveActive)
            return;
        
        // 1. Determine target heat from the stove setting
        float targetHeat = 70 + ((m_iStoveSetting / 10.0) * m_fMaxHeat);
        if (m_iStoveSetting == 11)  // If you had an "overdrive" check
            targetHeat = m_fMaxHeat * 1.15;

        // 2. Heat soak logic (unchanged)
        if (m_fCurrentStoveHeat > 100.0)
        {
            m_fHeatSoak += deltaTime * 0.000015 * m_fCurrentStoveHeat;
            if (m_fHeatSoak > 1.0)
                m_fHeatSoak = 1.0;
        }
        else
        {
            m_fHeatSoak -= deltaTime * 0.004; 
            if (m_fHeatSoak < 0.0)
                m_fHeatSoak = 0.0;
        }

        // 3. Adjust HeatSoak if needed
        float maxAllowedHeatSoak = (targetHeat - m_fAmbientTemperature) / m_fMaxHeat;
        if (m_fHeatSoak > maxAllowedHeatSoak)
        {
            m_fHeatSoak -= deltaTime * 0.02;
            if (m_fHeatSoak < maxAllowedHeatSoak)
                m_fHeatSoak = maxAllowedHeatSoak;
        }

        // 4. Equilibrium temperature logic
        float equilibriumTemperature = m_fAmbientTemperature + (m_fHeatSoak * m_fMaxHeat);
        if (equilibriumTemperature > targetHeat)
        {
            equilibriumTemperature -= deltaTime * (equilibriumTemperature - targetHeat) * 0.1;
            if (equilibriumTemperature < targetHeat)
                equilibriumTemperature = targetHeat;
        }

        // 5. Heating / cooling calculation
        float effectiveThermalInertia = m_fThermalInertia * (1.0 - 0.5 * m_fHeatSoak);
        float effectiveCoolingRate    = m_fCoolingRate   * (1.0 - 0.5 * m_fHeatSoak);

        float dampingFactor = (equilibriumTemperature - m_fCurrentStoveHeat) / m_fMaxHeat;
        if (dampingFactor < 0) 
            dampingFactor = -dampingFactor; // absolute value

        float heatGain = 0.0;
        float heatLoss = 0.0;

        if (targetHeat > m_fCurrentStoveHeat)
        {
            heatGain = (targetHeat - m_fCurrentStoveHeat) / effectiveThermalInertia;
        }
			
        if (equilibriumTemperature > m_fCurrentStoveHeat)
        {
            heatLoss = -effectiveCoolingRate * dampingFactor * (equilibriumTemperature - m_fCurrentStoveHeat);
        }
        else if (equilibriumTemperature < m_fCurrentStoveHeat)
        {
            heatLoss = effectiveCoolingRate * (m_fCurrentStoveHeat - equilibriumTemperature);
        }

        m_fCurrentStoveHeat += heatGain - heatLoss;
        if (m_fCurrentStoveHeat < 0)
            m_fCurrentStoveHeat = 0;
		
		
		        Print("[StoveSimulation] StoveHeat=" + m_fCurrentStoveHeat 
            + " | TargetHeat=" + targetHeat 
            + " | HeatSoak=" + m_fHeatSoak, LogLevel.NORMAL);
    
		
		if (m_fCurrentStoveHeat<200){
			GameSignal.SetSignalValue(BoilIndex, 0);
		}else{

			GameSignal.SetSignalValue(BoilIndex, (m_fCurrentStoveHeat-199)/300);
			
		}

	}

    float GetCurrentStoveHeat()
    {
        return m_fCurrentStoveHeat;
    }
    
    int GetStoveSetting()
    {
        return m_iStoveSetting;
    }
    
    bool IsStoveActive()
    {
        return m_bStoveActive;
    }
}
