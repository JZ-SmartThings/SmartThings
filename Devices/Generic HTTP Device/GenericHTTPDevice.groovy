/**
 *  Generic HTTP Device v1.0.20170227
 *  Source code can be found here: https://github.com/JZ-SmartThings/SmartThings/blob/master/Devices/Generic%20HTTP%20Device/GenericHTTPDevice.groovy
 *  Copyright 2017 JZ
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 */

import groovy.json.JsonSlurper

metadata {
	definition (name: "Generic HTTP Device", author: "JZ", namespace:"JZ") {
		capability "Switch"
		capability "Temperature Measurement"
		capability "Contact Sensor"
		capability "Sensor"
		capability "Polling"
		capability "Refresh"
		attribute "mainTriggered", "string"
		attribute "refreshTriggered", "string"
		attribute "customswitch", "string"
		attribute "customTriggered", "string"
		attribute "customTriggeredEPOCH", "number"
		attribute "cpuUsage", "string"
		attribute "spaceUsed", "string"
		attribute "upTime", "string"
		attribute "cpuTemp", "string"
		attribute "freeMem", "string"
		attribute "temperature", "string"
		attribute "humidity", "string"
		command "DeviceTrigger"
		command "RefreshTrigger"
		command "CustomTrigger"
		command "RebootNow"
		command "ResetTiles"
		command "ClearTiles"
	}

	preferences {
		input("DeviceIP", "string", title:"Device IP Address", description: "Please enter your device's IP Address", required: true, displayDuringSetup: true)
		input("DevicePort", "string", title:"Device Port", description: "Empty assumes port 80.", required: false, displayDuringSetup: true)
		input("DevicePath", "string", title:"URL Path", description: "Rest of the URL, include forward slash.", displayDuringSetup: true)
		input(name: "DevicePostGet", type: "enum", title: "POST or GET. POST for PHP & GET for Arduino.", options: ["POST","GET"], defaultValue: "POST", required: false, displayDuringSetup: true)
		input("UseOffVoiceCommandForCustom", "bool", title:"Use the OFF voice command (e.g. by Alexa) to control the Custom command? Assumed ON if MainTrigger is Momentary setting below is ON.", description: "", defaultValue: false, required: false, displayDuringSetup: true)
		input("DeviceMainMomentary", "bool", title:"MainTrigger is Momentary?", description: "", defaultValue: true, required: false, displayDuringSetup: true)	
		input("DeviceMainPin", "number", title:'Main Pin Number in BCM Format', description: 'Empty assumes pin #4.', required: false, displayDuringSetup: false)
		input("DeviceCustomMomentary", "bool", title:"CustomTrigger is Momentary?", description: "", defaultValue: true, required: false, displayDuringSetup: true)
		input("DeviceCustomPin", "number", title:'Custom Pin Number in BCM Format', description: 'Empty assumes pin #21.', required: false, displayDuringSetup: false)
		input("UseJSON", "bool", title:"Use JSON instead of HTML?", description: "", defaultValue: false, required: false, displayDuringSetup: true)
		section() {
			input("HTTPAuth", "bool", title:"Requires User Auth?", description: "Choose if the HTTP requires basic authentication", defaultValue: false, required: true, displayDuringSetup: true)
			input("HTTPUser", "string", title:"HTTP User", description: "Enter your basic username", required: false, displayDuringSetup: true)
			input("HTTPPassword", "string", title:"HTTP Password", description: "Enter your basic password", required: false, displayDuringSetup: true)
		}
	}
	
	simulator {
	}

	tiles(scale: 2) {
		valueTile("displayName", "device.displayName", width: 6, height: 1, decoration: "flat") {
			state("default", label: '${currentValue}', backgroundColor:"#DDDDDD")
		}
		valueTile("mainTriggered", "device.mainTriggered", width: 5, height: 1, decoration: "flat") {
			state("default", label: 'Main triggered:\r\n${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("DeviceTrigger", "device.switch", width: 1, height: 1, canChangeIcon: true, canChangeBackground: true, decoration: "flat") {
			state "off", label:'OFF' , action: "on", icon: "st.Outdoor.outdoor22", backgroundColor:"#53a7c0", nextState: "trying"
			state "on", label: 'ON', action: "on", icon: "st.Outdoor.outdoor22", backgroundColor: "#FF6600", nextState: "trying"
			state "trying", label: 'TRYING', action: "ResetTiles", icon: "st.Outdoor.outdoor22", backgroundColor: "#FFAA33"
		}
		valueTile("customTriggered", "device.customTriggered", width: 5, height: 1, decoration: "flat") {
			state("default", label: 'Custom triggered:\r\n${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("CustomTrigger", "device.customswitch", width: 1, height: 1, decoration: "flat") {
			state "off", label:'CUSTOM', action: "CustomTrigger", icon: "st.Lighting.light13", backgroundColor:"#53a7c0", nextState: "trying"
			state "on", label: 'ON', action: "CustomTrigger", icon: "st.Lighting.light11", backgroundColor: "#FF6600", nextState: "trying"
			state "trying", label: 'TRYING', action: "ResetTiles", icon: "st.Lighting.light11", backgroundColor: "#FFAA33"
		}
		valueTile("sensorTriggered", "device.sensorTriggered", width: 5, height: 1, decoration: "flat") {
			state("default", label: 'Sensor State Changed:\r\n${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("contact", "device.contact", width: 1, height: 1, decoration: "flat") {
			state "open", label: '${name}', icon: "st.contact.contact.open", backgroundColor: "#ffa81e"
			state "closed", label: '${name}', icon: "st.contact.contact.closed", backgroundColor: "#79b821"
		}
		valueTile("refreshTriggered", "device.refreshTriggered", width: 5, height: 1, decoration: "flat") {
			state("default", label: 'Refreshed:\r\n${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("RefreshTrigger", "device.refreshswitch", width: 1, height: 1, decoration: "flat") {
			state "default", label:'REFRESH', action: "refresh.refresh", icon: "st.secondary.refresh-icon", backgroundColor:"#53a7c0", nextState: "refreshing"
			state "refreshing", label: 'REFRESHING', action: "ResetTiles", icon: "st.secondary.refresh-icon", backgroundColor: "#FF6600", nextState: "default"
		}

		valueTile("cpuUsage", "device.cpuUsage", width: 2, height: 2) {
			state("default", label: 'CPU\r\n ${currentValue}%',
				backgroundColors:[
					[value: 0, color: "#00cc33"],
					[value: 10, color: "#99ff33"],
					[value: 30, color: "#ffcc99"],
					[value: 55, color: "#ff6600"],
					[value: 90, color: "#ff0000"]
				]
			)
		}
		valueTile("cpuTemp", "device.cpuTemp", width: 2, height: 2) {
			state("default", label: 'CPU Temp ${currentValue}',
				backgroundColors:[
					[value: 50, color: "#00cc33"],
					[value: 60, color: "#99ff33"],
					[value: 67, color: "#ff6600"],
					[value: 75, color: "#ff0000"]
				]
			)
		}
		valueTile("spaceUsed", "device.spaceUsed", width: 2, height: 2) {
			state("default", label: 'Space Used\r\n ${currentValue}%',
				backgroundColors:[
					[value: 50, color: "#00cc33"],
					[value: 75, color: "#ffcc66"],
					[value: 85, color: "#ff6600"],
					[value: 95, color: "#ff0000"]
				]
			)
		}
		valueTile("upTime", "device.upTime", width: 2, height: 2, decoration: "flat") {
			state("default", label: 'UpTime\r\n ${currentValue}', backgroundColor:"#ffffff")
		}
		valueTile("freeMem", "device.freeMem", width: 2, height: 2, decoration: "flat") {
			state("default", label: 'Free Mem\r\n ${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("clearTiles", "device.clear", width: 2, height: 2, decoration: "flat") {
			state "default", label:'Clear Tiles', action:"ClearTiles", icon:"st.Bath.bath9"
		}
		valueTile("temperature", "device.temperature", width: 2, height: 2) {
			state("default", label:'Temp\n ${currentValue}',
				backgroundColors:[
					[value: 32, color: "#153591"],
					[value: 44, color: "#1e9cbb"],
					[value: 59, color: "#90d2a7"],
					[value: 74, color: "#44b621"],
					[value: 84, color: "#f1d801"],
					[value: 92, color: "#d04e00"],
					[value: 98, color: "#bc2323"]
				]
			)
		}
		valueTile("humidity", "device.humidity", width: 2, height: 2) {
			state("default", label: 'Humidity\n ${currentValue}',
				backgroundColors:[
					[value: 50, color: "#00cc33"],
					[value: 60, color: "#99ff33"],
					[value: 67, color: "#ff6600"],
					[value: 75, color: "#ff0000"]
				]
			)
		}
		standardTile("RebootNow", "device.rebootnow", width: 1, height: 1, decoration: "flat") {
			state "default", label:'REBOOT' , action: "RebootNow", icon: "st.Seasonal Winter.seasonal-winter-014", backgroundColor:"#ff0000", nextState: "rebooting"
			state "rebooting", label: 'REBOOTING', action: "ResetTiles", icon: "st.Office.office13", backgroundColor: "#FF6600", nextState: "default"
		}
		main "DeviceTrigger"
		details(["displayName","mainTriggered", "DeviceTrigger", "customTriggered", "CustomTrigger", "sensorTriggered", "contact", "refreshTriggered", "RefreshTrigger", "cpuUsage", "cpuTemp", "upTime", "spaceUsed", "freeMem", "clearTiles", "temperature", "humidity" , "RebootNow"])
	}
}

def refresh() {
	def FullCommand = 'Refresh='
	if (DeviceMainPin) {FullCommand=FullCommand+"&MainPin="+DeviceMainPin} //else {FullCommand=FullCommand+"&MainPin=4"}
	if (DeviceCustomPin) {FullCommand=FullCommand+"&CustomPin="+DeviceCustomPin} //else {FullCommand=FullCommand+"&CustomPin=21"}
	if (UseJSON==true) { FullCommand=FullCommand+"&UseJSON=" }
	if (DeviceMainMomentary==true) { settings.UseOffVoiceCommandForCustom = true }
	runCmd(FullCommand)
}
def poll() {
	refresh()
}
def on() {
	def FullCommand = ''
	if (DeviceMainMomentary==true) {
		settings.UseOffVoiceCommandForCustom = true
		FullCommand='MainTrigger='
	} else {
		if (device.currentState("switch")!=null && device.currentState("switch").getValue()=="off") { FullCommand='MainTriggerOn=' } else { FullCommand='MainTriggerOff=' }
	}
	if (DeviceMainPin) {FullCommand=FullCommand+"&MainPin="+DeviceMainPin} //else {FullCommand=FullCommand+"&MainPin=4"}
	if (DeviceCustomPin) {FullCommand=FullCommand+"&CustomPin="+DeviceCustomPin} //else {FullCommand=FullCommand+"&CustomPin=21"}
	if (UseJSON==true) {FullCommand=FullCommand+"&UseJSON="}
	runCmd(FullCommand)
}
def off() {
	if (DeviceMainMomentary==true || UseOffVoiceCommandForCustom==true) {
		settings.UseOffVoiceCommandForCustom = true
		CustomTrigger()
	} else {
		log.debug "Running ON() Function for MAIN Command Handling."
		on()
	}
}

def CustomTrigger() {
	//log.debug device.currentState("customswitch").getValue() + " === customswitch state"
	def FullCommand = ''
	if (DeviceCustomMomentary==true) {
		settings.UseOffVoiceCommandForCustom = true
		FullCommand='CustomTrigger='
	} else {
		log.debug "main swtich currentState===" + device.currentState("switch")
		if (device.currentState("switch")!=null && device.currentState("customswitch").getValue()=="off") { FullCommand='CustomTriggerOn=' } else { FullCommand='CustomTriggerOff=' }
	}
	if (DeviceMainPin) {FullCommand=FullCommand+"&MainPin="+DeviceMainPin} //else {FullCommand=FullCommand+"&MainPin=4"}
	if (DeviceCustomPin) {FullCommand=FullCommand+"&CustomPin="+DeviceCustomPin} //else {FullCommand=FullCommand+"&CustomPin=21"}
	if (UseJSON==true) {FullCommand=FullCommand+"&UseJSON="}
	runCmd(FullCommand)
}
def RebootNow() {
	log.debug "Reboot Triggered!!!"
	runCmd('RebootNow=')
}
def ClearTiles() {
	sendEvent(name: "mainTriggered", value: "", unit: "")
	sendEvent(name: "customTriggered", value: "", unit: "")
	sendEvent(name: "refreshTriggered", value: "", unit: "")
	sendEvent(name: "sensorTriggered", value: "", unit: "")
	sendEvent(name: "cpuUsage", value: "", unit: "")
	sendEvent(name: "cpuTemp", value: "", unit: "")
	sendEvent(name: "spaceUsed", value: "", unit: "")
	sendEvent(name: "upTime", value: "", unit: "")
	sendEvent(name: "freeMem", value: "", unit: "")
	sendEvent(name: "temperature", value: "", unit: "")
	sendEvent(name: "humidity", value: "", unit: "")
}
def ResetTiles() {
	//RETURN BUTTONS TO CORRECT STATE
	if (DeviceMainMomentary==true) {
		sendEvent(name: "switch", value: "off", isStateChange: true)
	}
	if (DeviceCustomMomentary==true) {
		sendEvent(name: "customswitch", value: "off", isStateChange: true)
	}
	sendEvent(name: "refreshswitch", value: "default", isStateChange: true)
	sendEvent(name: "rebootnow", value: "default", isStateChange: true)
	log.debug "Resetting tiles."
}

def runCmd(String varCommand) {
	def host = DeviceIP
	def hosthex = convertIPtoHex(host).toUpperCase()
	def LocalDevicePort = ''
	if (DevicePort==null) { LocalDevicePort = "80" } else { LocalDevicePort = DevicePort }
	def porthex = convertPortToHex(LocalDevicePort).toUpperCase()
	device.deviceNetworkId = "$hosthex:$porthex"
	def userpassascii = "${HTTPUser}:${HTTPPassword}"
	def userpass = "Basic " + userpassascii.encodeAsBase64().toString()

	log.debug "The device id configured is: $device.deviceNetworkId"

	def headers = [:] 
	headers.put("HOST", "$host:$LocalDevicePort")
	headers.put("Content-Type", "application/x-www-form-urlencoded")
	if (HTTPAuth) {
		headers.put("Authorization", userpass)
	}
	log.debug "The Header is $headers"

	def path = ''
	def body = ''
	log.debug "Uses which method: $DevicePostGet"
	def method = "POST"
	try {
		if (DevicePostGet.toUpperCase() == "GET") {
			method = "GET"
			path = varCommand
			if (path.substring(0,1) != "/") { path = "/" + path }
			log.debug "GET path is: $path"
		} else {
			path = DevicePath
			body = varCommand 
			log.debug "POST body is: $body"
		}
		log.debug "The method is $method"
	}
	catch (Exception e) {
		settings.DevicePostGet = "POST"
		log.debug e
		log.debug "You must not have set the preference for the DevicePOSTGET option"
	}

	try {
		def hubAction = new physicalgraph.device.HubAction(
			method: method,
			path: path,
			body: body,
			headers: headers
			)
		hubAction.options = [outputMsgToS3:false]
		log.debug hubAction
		hubAction
	}
	catch (Exception e) {
		log.debug "Hit Exception $e on $hubAction"
	}
}

def parse(String description) {
//	log.debug "Parsing '${description}'"
	def whichTile = ''
	def map = [:]
	def retResult = []
	def descMap = parseDescriptionAsMap(description)
	def jsonlist = [:]
	def bodyReturned = ' '
	def headersReturned = ' '
	if (descMap["body"]) { bodyReturned = new String(descMap["body"].decodeBase64()) }
	if (descMap["headers"]) { headersReturned = new String(descMap["headers"].decodeBase64()) }
	//log.debug "BODY---" + bodyReturned
	//log.debug "HEADERS---" + headersReturned

	if (descMap["body"]) {
		if (headersReturned.contains("application/json")) {
			def body = new String(descMap["body"].decodeBase64())
			def slurper = new JsonSlurper()
			jsonlist = slurper.parseText(body)
			//log.debug "JSONLIST---" + jsonlist."CPU"
			jsonlist.put ("Date", new Date().format("yyyy-MM-dd h:mm:ss a", location.timeZone))
		} else {
			jsonlist.put ("Date", new Date().format("yyyy-MM-dd h:mm:ss a", location.timeZone))
			def data=bodyReturned.eachLine { line ->
				if (line.contains('CPU=')) { jsonlist.put ("CPU", line.replace("CPU=","")) }
				if (line.contains('Space Used=')) { jsonlist.put ("Space Used", line.replace("Space Used=","")) }
				if (line.contains('UpTime=')) { jsonlist.put ("UpTime", line.replace("UpTime=","")) }
				if (line.contains('CPU Temp=')) { jsonlist.put ("CPU Temp",line.replace("CPU Temp=","")) }
				if (line.contains('Free Mem=')) { jsonlist.put ("Free Mem",line.replace("Free Mem=",""))  }
				if (line.contains('Temperature=')) { jsonlist.put ("Temperature",line.replace("Temperature=","").replaceAll("[^\\p{ASCII}]", "°")) }
				if (line.contains('Humidity=')) { jsonlist.put ("Humidity",line.replace("Humidity=","")) }
				if (line.contains('MainTrigger=Success')) { jsonlist.put ("MainTrigger".replace("=",""), "Success") }
				if (line.contains('MainTrigger=Failed : Authentication Required!')) { jsonlist.put ("MainTrigger".replace("=",""), "Authentication Required!") }
				if (line.contains('MainTriggerOn=Success')) { jsonlist.put ("MainTriggerOn", "Success") }
				if (line.contains('MainTriggerOn=Failed : Authentication Required!')) { jsonlist.put ("MainTriggerOn", "Authentication Required!") }
				if (line.contains('MainTriggerOff=Success')) { jsonlist.put ("MainTriggerOff", "Success") }
				if (line.contains('MainTriggerOff=Failed : Authentication Required!')) { jsonlist.put ("MainTriggerOff", "Authentication Required!") }
				if (line.contains('MainPinStatus=1')) { jsonlist.put ("MainPinStatus".replace("=",""), 1) }
				if (line.contains('MainPinStatus=0')) { jsonlist.put ("MainPinStatus".replace("=",""), 0) }
				if (line.contains('CustomTrigger=Success')) { jsonlist.put ("CustomTrigger", "Success") }
				if (line.contains('CustomTrigger=Failed : Authentication Required!')) { jsonlist.put ("CustomTrigger", "Authentication Required!") }
				if (line.contains('CustomTriggerOn=Success')) { jsonlist.put ("CustomTriggerOn", "Success") }
				if (line.contains('CustomTriggerOn=Failed : Authentication Required!')) { jsonlist.put ("CustomTriggerOn", "Authentication Required!") }
				if (line.contains('CustomTriggerOff=Success')) { jsonlist.put ("CustomTriggerOff", "Success") }
				if (line.contains('CustomTriggerOff=Failed : Authentication Required!')) { jsonlist.put ("CustomTriggerOff", "Authentication Required!") }
				if (line.contains('CustomPinStatus=1')) { jsonlist.put ("CustomPinStatus".replace("=",""), 1) }
				if (line.contains('CustomPinStatus=0')) { jsonlist.put ("CustomPinStatus".replace("=",""), 0) }
				if (line.contains('SensorPinStatus=Open')) { jsonlist.put ("SensorPinStatus".replace("=",""), "Open") }
				if (line.contains('SensorPinStatus=Closed')) { jsonlist.put ("SensorPinStatus".replace("=",""), "Closed") }
				if (line.contains('Contact Sensor=Open')) { jsonlist.put ("SensorPinStatus".replace("=",""), "Open") }
				if (line.contains('Contact Sensor=Closed')) { jsonlist.put ("SensorPinStatus".replace("=",""), "Closed") }
				if (line.contains('Refresh=Success')) { jsonlist.put ("Refresh", "Success") }
				if (line.contains('Refresh=Failed : Authentication Required!')) { jsonlist.put ("Refresh", "Authentication Required!") }
				if (line.contains('RebootNow=Success')) { jsonlist.put ("RebootNow", "Success") }
				if (line.contains('RebootNow=Failed : Authentication Required!')) { jsonlist.put ("RebootNow", "Authentication Required!") }
				//ARDUINO CHECKS
				if (line.contains('/MainTrigger=')) { jsonlist.put ("MainTrigger".replace("=",""), "Success") }
				if (line.contains('/MainTriggerOn=')) { jsonlist.put ("MainTriggerOn", "Success") }
				if (line.contains('/MainTriggerOff=')) { jsonlist.put ("MainTriggerOff", "Success") }
				if (line.contains('RELAY1 pin is now: On')) { jsonlist.put ("MainPinStatus".replace("=",""), 1) }
				if (line.contains('RELAY1 pin is now: Off')) { jsonlist.put ("MainPinStatus".replace("=",""), 0) }
				if (line.contains('/CustomTrigger=')) { jsonlist.put ("CustomTrigger".replace("=",""), "Success") }
				if (line.contains('/CustomTriggerOn=')) { jsonlist.put ("CustomTriggerOn", "Success") }
				if (line.contains('/CustomTriggerOff=')) { jsonlist.put ("CustomTriggerOff", "Success") }
				if (line.contains('RELAY2 pin is now: On')) { jsonlist.put ("CustomPinStatus".replace("=",""), 1) }
				if (line.contains('RELAY2 pin is now: Off')) { jsonlist.put ("CustomPinStatus".replace("=",""), 0) }
				if (line == '/Refresh=') { jsonlist.put ("Refresh", "Success") }
			}
		}
	}
	if (descMap["body"]) {
		if (jsonlist."Refresh"=="Authentication Required!") {
			sendEvent(name: "refreshTriggered", value: "Use Authentication Credentials", unit: "")
			whichTile = 'refresh'
		}
		if (jsonlist."Refresh"=="Success") {
			sendEvent(name: "refreshTriggered", value: jsonlist."Date", unit: "")
			whichTile = 'refresh'
		}
		if (jsonlist."CustomTrigger"=="Authentication Required!") {
			sendEvent(name: "customTriggered", value: "Use Authentication Credentials", unit: "")
		}
		if (jsonlist."CustomTrigger"=="Success") {
			sendEvent(name: "customswitch", value: "on", isStateChange: true)
			sendEvent(name: "customTriggered", value: "MOMENTARY @ " + jsonlist."Date", unit: "")
            sendEvent(name: "customTriggeredEPOCH", value: now(), isStateChange: true)
			whichTile = 'customoff'
		}
		if (jsonlist."CustomTriggerOn"=="Success" && jsonlist."CustomPinStatus"==1) {
			sendEvent(name: "customTriggered", value: "ON @ " + jsonlist."Date", unit: "")
            sendEvent(name: "customTriggeredEPOCH", value: now(), isStateChange: true)
			whichTile = 'customon'
		}
		if (jsonlist."CustomTriggerOn"=="Authentication Required!") {
			sendEvent(name: "customTriggered", value: "Use Authentication Credentials", unit: "")
		}
		if (jsonlist."CustomTriggerOff"=="Success" && jsonlist."CustomPinStatus"==0) {
			sendEvent(name: "customTriggered", value: "OFF @ " + jsonlist."Date", unit: "")
            sendEvent(name: "customTriggeredEPOCH", value: now(), isStateChange: true)
			whichTile = 'customoff'
		}
		if (jsonlist."CustomTriggerOff"=="Authentication Required!") {
			sendEvent(name: "customTriggered", value: "Use Authentication Credentials", unit: "")
		}
		if (jsonlist."CustomPinStatus"==1) {
			sendEvent(name: "customswitch", value: "on", isStateChange: true)
            sendEvent(name: "customTriggeredEPOCH", value: now(), isStateChange: true)
			sendEvent(name: "refreshswitch", value: "default", isStateChange: true)
			whichTile = 'customon'
		} else if (jsonlist."CustomPinStatus"==0) {
			sendEvent(name: "customswitch", value: "off", isStateChange: true)
            sendEvent(name: "customTriggeredEPOCH", value: now(), isStateChange: true)
			sendEvent(name: "refreshswitch", value: "default", isStateChange: true)
			whichTile = 'customoff'
		}
		if (jsonlist."MainTrigger"=="Authentication Required!") {
			sendEvent(name: "mainTriggered", value: "Use Authentication Credentials", unit: "")
		}
		if (jsonlist."MainTrigger"=="Success") {
			sendEvent(name: "switch", value: "on", isStateChange: true)
			sendEvent(name: "mainTriggered", value: "MOMENTARY @ " + jsonlist."Date", unit: "")
			whichTile = 'mainoff'
		}
		if (jsonlist."MainTriggerOn"=="Success" && jsonlist."MainPinStatus"==1) {
			sendEvent(name: "mainTriggered", value: "ON @ " + jsonlist."Date", unit: "")
			whichTile = 'mainon'
		}
		if (jsonlist."MainTriggerOn"=="Authentication Required!") {
			sendEvent(name: "mainTriggered", value: "Use Authentication Credentials", unit: "")
		}
		if (jsonlist."MainTriggerOff"=="Success" && jsonlist."MainPinStatus"==0) {
			sendEvent(name: "mainTriggered", value: "OFF @ " + jsonlist."Date", unit: "")
			whichTile = 'mainoff'
		}
		if (jsonlist."MainTriggerOff"=="Authentication Required!") {
			sendEvent(name: "mainTriggered", value: "Use Authentication Credentials", unit: "")
			whichTile = 'mainoff'
		}
		if (jsonlist."MainPinStatus"==1) {
			sendEvent(name: "switch", value: "on", isStateChange: true)
			sendEvent(name: "refreshswitch", value: "default", isStateChange: true)
			whichTile = 'mainon'
		} else if (jsonlist."MainPinStatus"==0) {
			sendEvent(name: "switch", value: "off", isStateChange: true)
			sendEvent(name: "refreshswitch", value: "default", isStateChange: true)
			whichTile = 'mainoff'
		}
		if (jsonlist."SensorPinStatus"=="Open") {
			if (device.currentState("contact").getValue()=="closed") { sendEvent(name: "sensorTriggered", value: "OPEN @ " + jsonlist."Date", unit: "") }
			sendEvent(name: "contact", value: "open", descriptionText: "$device.displayName is open")
			sendEvent(name: "refreshswitch", value: "default", isStateChange: true)
		} else if (jsonlist."SensorPinStatus"=="Closed") {
			if (device.currentState("contact").getValue()=="open") { sendEvent(name: "sensorTriggered", value: "CLOSED @ " + jsonlist."Date", unit: "") }
			sendEvent(name: "contact", value: "closed", descriptionText: "$device.displayName is closed")
			sendEvent(name: "refreshswitch", value: "default", isStateChange: true)
		} else {
			sendEvent(name: "contact", value: "closed", descriptionText: "$device.displayName is closed")
			sendEvent(name: "refreshswitch", value: "default", isStateChange: true)
		}
		if (jsonlist."CPU") {
			sendEvent(name: "cpuUsage", value: jsonlist."CPU".replace("=","\n").replace("%",""), unit: "")
		}
		if (jsonlist."Space Used") {
			sendEvent(name: "spaceUsed", value: jsonlist."Space Used".replace("=","\n").replace("%",""), unit: "")
		}
		if (jsonlist."UpTime") {
			sendEvent(name: "upTime", value: jsonlist."UpTime".replace("=","\n"), unit: "")
		}
		if (jsonlist."CPU Temp") {
			sendEvent(name: "cpuTemp", value: jsonlist."CPU Temp".replace("=","\n").replace("\'","°").replace("C ","C="), unit: "")
		}
		if (jsonlist."Free Mem") {
			sendEvent(name: "freeMem", value: jsonlist."Free Mem".replace("=","\n"), unit: "")
		}
		if (jsonlist."Temperature") {
			sendEvent(name: "temperature", value: jsonlist."Temperature".replace("=","\n").replace("\'","°").replace("C ","C="), unit: "")
			//String s = jsonlist."Temperature"
			//for(int i = 0; i < s.length(); i++)	{
			//   int c = s.charAt(i);
			//   log.trace "'${c}'\n"
			//}
		}
		if (jsonlist."Humidity") {
			sendEvent(name: "humidity", value: jsonlist."Humidity".replace("=","\n"), unit: "")
		}
		if (jsonlist."RebootNow") {
			whichTile = 'RebootNow'
		}
	}

	log.debug jsonlist

	//RESET THE DEVICE ID TO GENERIC/RANDOM NUMBER. THIS ALLOWS MULTIPLE DEVICES TO USE THE SAME ID/IP
	device.deviceNetworkId = "ID_WILL_BE_CHANGED_AT_RUNTIME_" + (Math.abs(new Random().nextInt()) % 99999 + 1)

	//CHANGE NAME TILE
	sendEvent(name: "displayName", value: DeviceIP, unit: "")

	//RETURN BUTTONS TO CORRECT STATE
	log.debug 'whichTile: ' + whichTile
    switch (whichTile) {
        case 'refresh':
			sendEvent(name: "refreshswitch", value: "default", isStateChange: true)
			def result = createEvent(name: "refreshswitch", value: "default", isStateChange: true)
			//log.debug "refreshswitch returned ${result?.descriptionText}"
			return result
        case 'customoff':
			sendEvent(name: "customswitch", value: "off", isStateChange: true)
			def result = createEvent(name: "customswitch", value: "off", isStateChange: true)
			return result
        case 'customon':
			sendEvent(name: "customswitch", value: "on", isStateChange: true)
			def result = createEvent(name: "customswitch", value: "on", isStateChange: true)
			return result
        case 'mainoff':
			def result = createEvent(name: "switch", value: "off", isStateChange: true)
			return result
        case 'mainon':
			def result = createEvent(name: "switch", value: "on", isStateChange: true)
			return result
        case 'RebootNow':
			sendEvent(name: "rebootnow", value: "default", isStateChange: true)
			def result = createEvent(name: "rebootnow", value: "default", isStateChange: true)
			return result
        default:
			sendEvent(name: "refreshswitch", value: "default", isStateChange: true)
			def result = createEvent(name: "refreshswitch", value: "default", isStateChange: true)
			//log.debug "refreshswitch returned ${result?.descriptionText}"
			return result
    }
}

def parseDescriptionAsMap(description) {
	description.split(",").inject([:]) { map, param ->
	def nameAndValue = param.split(":")
	map += [(nameAndValue[0].trim()):nameAndValue[1].trim()]
	}
}
private String convertIPtoHex(ipAddress) { 
	String hex = ipAddress.tokenize( '.' ).collect {  String.format( '%02x', it.toInteger() ) }.join()
	//log.debug "IP address entered is $ipAddress and the converted hex code is $hex"
	return hex
}
private String convertPortToHex(port) {
	String hexport = port.toString().format( '%04x', port.toInteger() )
	//log.debug hexport
	return hexport
}
private Integer convertHexToInt(hex) {
	Integer.parseInt(hex,16)
}
private String convertHexToIP(hex) {
	//log.debug("Convert hex to ip: $hex") 
	[convertHexToInt(hex[0..1]),convertHexToInt(hex[2..3]),convertHexToInt(hex[4..5]),convertHexToInt(hex[6..7])].join(".")
}
private getHostAddress() {
	def parts = device.deviceNetworkId.split(":")
	//log.debug device.deviceNetworkId
	def ip = convertHexToIP(parts[0])
	def port = convertHexToInt(parts[1])
	return ip + ":" + port
}
