' Create a reference to the InstallShield automation layer
    Set m_ISWiProject = CreateObject("ISWiAutomation.ISWiProject")
    ' Convert .isv to .ism
    m_ISWiProject.ImportProject "BINARYPROJECT", "TEXTPROJECT"
