<?xml version="1.0"?>
<!-- This XSL transformation is used for generating DUCK config file for EmuDAQ and EmuDQM from a RUI-to-computer mapping. -->
<!-- Usage example:  -->
<!--     xsltproc [<hyphen><hyphen>stringparam SIDE 'P|M|B'] <hyphen><hyphen>stringparam NAME 'DAQ' EmuDAQDUCKGenerator.xsl RUI-to-computer_mapping.xml > DAQ.duck -->
<xsl:transform xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="1.0" xmlns:xs="http://www.w3.org/2001/XMLSchema"
  xmlns:xc="http://xdaq.web.cern.ch/xdaq/xsd/2004/XMLConfiguration-30"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
  xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/">

  <!-- Parameter SIDE is to be optionally set from the command line. -->
  <!-- If it's set to 'M', only the minus side will be generated; if 'P', only the plus side; if 'B', both sides; otherwise both sides. -->
  <xsl:param name="SIDE"/>
  <!-- Parameter NAME is to be set from the command line. -->
  <!-- It's the name of the configuration (without path or extension) -->
  <xsl:param name="NAME"/>

  <xsl:param name="CONFIG_FILE">/nfshome0/cscdaq/config/merged/<xsl:value-of select="$NAME"/>.xml</xsl:param>
  <xsl:param name="FM_CONFIG_PATH">DAQ/<xsl:value-of select="$NAME"/></xsl:param>

  <xsl:output method="xml" indent="yes"/>

  <!-- Generate DUCK file for EmuDAQ and EmuDQM -->
  <xsl:template match="RUI-to-computer_mapping">
    <xsl:comment>Generated by EmuDAQDUCKGenerator.xsl from RUI-to-computer_mapping.xml of <xsl:value-of select="@date"/></xsl:comment>
    <Configuration xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" user="cscpro" path="{$FM_CONFIG_PATH}">
      <FunctionManager name="config" hostname="cmsrc-csc.cms" port="12000"
		       qualifiedResourceType="rcms.fm.resource.qualifiedresource.FunctionManager"
		       sourceURL="http://cmsrc-csc.cms:12000/functionmanagers/cscFM.jar"
		       className="rcms.fm.app.csc.CSCFunctionManager"
		       role="CSC" >
	<xsl:call-template name="JobControls"/>
	<xsl:call-template name="DAQManager"/>
	<xsl:call-template name="EVM_and_TA"/>
	<xsl:call-template name="TF"/>
	<xsl:call-template name="RUIs"/>
	<xsl:call-template name="DisplayClient"/>
	<xsl:call-template name="Monitors"/>
      </FunctionManager>
    </Configuration>
  </xsl:template>

  <!-- EmuDAQManager -->
  <xsl:template name="DAQManager">
    <xsl:comment>EmuDAQManager</xsl:comment>
    <XdaqExecutive hostname="csc-daq00.cms" port="40200"
		   urn="urn:xdaq-application:lid=0"
		   qualifiedResourceType="rcms.fm.resource.qualifiedresource.XdaqExecutive"
		   instance="0"
		   pathToExecutive="/opt/xdaq/bin/xdaq.exe"
		   unixUser="cscdaq"
		   logLevel="WARN"
		   logURL="file:/tmp/xdaq-daqmanager-cscdaq.log"
		   environmentString="HOME=/nfshome0/cscdaq LD_LIBRARY_PATH=/nfshome0/cscdaq/TriDAS/x86/lib:/opt/xdaq/lib XDAQ_ROOT=/opt/xdaq BUILD_HOME=/nfshome0/cscdaq/TriDAS XDAQ_DOCUMENT_ROOT=/opt/xdaq/htdocs XDAQ_PLATFORM=x86 XDAQ_OS=linux PATH=/bin:/usr/bin">
      <configFile location="file"><xsl:value-of select="$CONFIG_FILE"/></configFile>
    </XdaqExecutive>
    <XdaqApplication className="EmuDAQManager" hostname="csc-daq00.cms" port="40200"
		     urn="urn:xdaq-application:lid=12"
		     qualifiedResourceType="rcms.fm.resource.qualifiedresource.XdaqApplication"
		     instance="0" />
  </xsl:template>

  <!-- EVM and EmuTA -->
  <xsl:template name="EVM_and_TA">
    <xsl:comment >EVM and EmuTA</xsl:comment>
    <XdaqExecutive hostname="csc-daq00.cms" port="40201"
		   urn="urn:xdaq-application:lid=0"
		   qualifiedResourceType="rcms.fm.resource.qualifiedresource.XdaqExecutive"
		   instance="0"
		   pathToExecutive="/opt/xdaq/bin/xdaq.exe"
		   unixUser="cscdaq"
		   logLevel="WARN"
		   logURL="file:/tmp/xdaq-evm_ta-cscdaq.log"
		   environmentString="HOME=/nfshome0/cscdaq LD_LIBRARY_PATH=/nfshome0/cscdaq/TriDAS/x86/lib:/opt/xdaq/lib XDAQ_ROOT=/opt/xdaq BUILD_HOME=/nfshome0/cscdaq/TriDAS XDAQ_DOCUMENT_ROOT=/opt/xdaq/htdocs XDAQ_PLATFORM=x86 XDAQ_OS=linux">
      <configFile location="file"><xsl:value-of select="$CONFIG_FILE"/></configFile>
    </XdaqExecutive>
  </xsl:template>

  <!-- Track Finder's RUI -->
  <xsl:template name="TF">
    <xsl:comment >RUI 0 (TF)</xsl:comment>
    <XdaqExecutive hostname="{//RUI[@instance='0']/../@alias}" port="{//RUI[@instance='0']/@port}"
		   urn="urn:xdaq-application:lid=0"
		   qualifiedResourceType="rcms.fm.resource.qualifiedresource.XdaqExecutive"
		   instance="0"
		   pathToExecutive="/opt/xdaq/bin/xdaq.exe"
		   unixUser="cscdaq"
		   logLevel="WARN"
		   logURL="file:/tmp/xdaq-rui0-cscdaq.log"
		   environmentString="HOME=/nfshome0/cscdaq LD_LIBRARY_PATH=/nfshome0/cscdaq/TriDAS/x86/lib:/opt/xdaq/lib XDAQ_ROOT=/opt/xdaq BUILD_HOME=/nfshome0/cscdaq/TriDAS XDAQ_DOCUMENT_ROOT=/opt/xdaq/htdocs XDAQ_PLATFORM=x86 XDAQ_OS=linux">
      <configFile location="file"><xsl:value-of select="$CONFIG_FILE"/></configFile>
    </XdaqExecutive>
  </xsl:template>

  <!-- RUIs -->
  <xsl:template name="RUIs">
    <xsl:for-each select="//RUI[@instance!='0']">
      <xsl:if test="($SIDE!='P' and $SIDE!='M') or $SIDE='B' or ($SIDE='P' and number(@instance)&lt;=18) or ($SIDE='M' and number(@instance)&gt;18)">
	
	<xsl:comment >RUI <xsl:value-of select="@instance"/></xsl:comment>
	<XdaqExecutive hostname="{../@alias}" port="{@port}"
		       urn="urn:xdaq-application:lid=0"
		       qualifiedResourceType="rcms.fm.resource.qualifiedresource.XdaqExecutive"
		       instance="0"
		       pathToExecutive="/opt/xdaq/bin/xdaq.exe"
		       unixUser="cscdaq"
		       logLevel="WARN"
		       logURL="file:/tmp/xdaq-rui{@instance}-cscdaq.log"
		       environmentString="HOME=/nfshome0/cscdaq LD_LIBRARY_PATH=/nfshome0/cscdaq/TriDAS/x86/lib:/opt/xdaq/lib XDAQ_ROOT=/opt/xdaq BUILD_HOME=/nfshome0/cscdaq/TriDAS XDAQ_DOCUMENT_ROOT=/opt/xdaq/htdocs XDAQ_PLATFORM=x86 XDAQ_OS=linux">
	  <configFile location="file"><xsl:value-of select="$CONFIG_FILE"/></configFile>
	</XdaqExecutive>

      </xsl:if>
    </xsl:for-each>
  </xsl:template>

  <!-- DQM Display Client -->
  <xsl:template name="DisplayClient">
    <xsl:comment >DQM Display Client</xsl:comment>
    <XdaqExecutive hostname="csc-dqm.cms" port="40550"
		   urn="urn:xdaq-application:lid=0"
		   qualifiedResourceType="rcms.fm.resource.qualifiedresource.XdaqExecutive"
		   instance="0"
		   pathToExecutive="/opt/xdaq/bin/xdaq.exe"
		   unixUser="cscdqm"
		   logLevel="INFO"
		   logURL="file:/tmp/xdaq-rui0-cscdqm.log"
		   environmentString="HOME=/nfshome0/cscdqm ROOTSYS=/nfshome0/cscdqm/root LD_LIBRARY_PATH=/nfshome0/cscdqm/root/lib:/opt/xdaq/lib XDAQ_ROOT=/opt/xdaq BUILD_HOME=/nfshome0/cscdqm/TriDAS XDAQ_DOCUMENT_ROOT=/opt/xdaq/htdocs XDAQ_PLATFORM=x86 XDAQ_OS=linux">
      <configFile location="file"><xsl:value-of select="$CONFIG_FILE"/></configFile>
    </XdaqExecutive>

  </xsl:template>

  <!-- DQM Monitors -->
  <xsl:template name="Monitors">
    <xsl:for-each select="//RUI">
      <xsl:if test="($SIDE!='P' and $SIDE!='M') or $SIDE='B' or ($SIDE='P' and number(@instance)&lt;=18) or ($SIDE='M' and number(@instance)&gt;18)">

	<xsl:variable name="PORT"><xsl:value-of select="40500+number(@instance)"/></xsl:variable>
	<xsl:comment >DQM Monitor <xsl:value-of select="@instance"/></xsl:comment>
	<XdaqExecutive hostname="{../@alias}" port="{$PORT}"
		       urn="urn:xdaq-application:lid=0"
		       qualifiedResourceType="rcms.fm.resource.qualifiedresource.XdaqExecutive"
		       instance="0"
		       pathToExecutive="/opt/xdaq/bin/xdaq.exe"
		       unixUser="cscdqm"
		       logLevel="WARN"
		       logURL="file:/tmp/xdaq-rui{@instance}-cscdqm.log"
		       environmentString="HOME=/nfshome0/cscdqm ROOTSYS=/nfshome0/cscdqm/root LD_LIBRARY_PATH=/nfshome0/cscdqm/root/lib:/opt/xdaq/lib XDAQ_ROOT=/opt/xdaq BUILD_HOME=/nfshome0/cscdqm/TriDAS XDAQ_DOCUMENT_ROOT=/opt/xdaq/htdocs XDAQ_PLATFORM=x86 XDAQ_OS=linux">
	  <configFile location="file"><xsl:value-of select="$CONFIG_FILE"/></configFile>
	</XdaqExecutive>

      </xsl:if>	
    </xsl:for-each>
  </xsl:template>

  <!-- JobControls -->
  <xsl:template name="JobControls">
    <xsl:comment>JobControls</xsl:comment>
    <xsl:for-each select="//RUI[not(../@alias = preceding::RUI/../@alias) and not(string-length(../@alias)=0)]">
      <Service name="JobControl" hostname="{../@alias}" port="39999" urn="urn:xdaq-application:lid=10" qualifiedResourceType="rcms.fm.resource.qualifiedresource.JobControl"/>
    </xsl:for-each>
    <xsl:for-each select="document('')//XdaqExecutive[not(@hostname = preceding::XdaqExecutive/@hostname)]">
      <xsl:if test="not(contains(@hostname,'{'))">
      <Service name="JobControl" hostname="{@hostname}" port="39999" urn="urn:xdaq-application:lid=10" qualifiedResourceType="rcms.fm.resource.qualifiedresource.JobControl"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

</xsl:transform>
