/**
 *  SmartGPIO v1.0.20160410
 *
 *  Source code can be found here: https://github.com/JZ-SmartThings/SmartThings/blob/master/Devices/SmartGPIO/SmartGPIO.groovy
 *
 *  Copyright 2016 JZ
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 *
 */

import groovy.json.JsonSlurper

metadata {
	definition (name: "SmartGPIO", author: "JZ", namespace:"JZ") {
		capability "Switch"
		capability "Polling"
		capability "Refresh"
		capability "Image Capture"
		attribute "lastTriggered", "string"
		attribute "testTriggered", "string"
		command "DeviceTrigger"
		command "TestTrigger"
		command "install"
	}

	preferences {
		input("DeviceIP", "string", title:"Device IP Address", description: "Please enter your device's IP Address", required: true, displayDuringSetup: true)
		input("DevicePort", "string", title:"Device Port", description: "Empty assumes port 80.", required: false, displayDuringSetup: true)
		input("DevicePath", "string", title:"URL Path", description: "Rest of the URL, include forward slash.", displayDuringSetup: true)
		input(name: "DevicePostGet", type: "enum", title: "POST or GET", options: ["POST","GET"], defaultValue: "POST", required: false, displayDuringSetup: true)
		input("UseJSON", "bool", title:"Use JSON instead of HTML?", description: "Use JSON instead of HTML?", defaultValue: false, required: false, displayDuringSetup: true)
		section() {
			input("HTTPAuth", "bool", title:"Requires User Auth?", description: "Choose if the HTTP requires basic authentication", defaultValue: false, required: true, displayDuringSetup: true)
			input("HTTPUser", "string", title:"HTTP User", description: "Enter your basic username", required: false, displayDuringSetup: true)
			input("HTTPPassword", "string", title:"HTTP Password", description: "Enter your basic password", required: false, displayDuringSetup: true)
		}
	}
	
	simulator {
	}

	tiles(scale: 2) {
		valueTile("testTriggered", "device.testTriggered", width: 5, height: 1, decoration: "flat") {
			state("default", label: 'Test triggered: ${currentValue}', backgroundColor:"#ffffff")
		}
		standardTile("TestTrigger", "device.testswitch", width: 1, height: 1, decoration: "flat") {
			state "default", label:'TEST', action: "poll", icon: "st.Office.office13", backgroundColor:"#53a7c0", nextState: "testrunning"
			state "testrunning", label: 'TESTING', action: "ResetTiles", icon: "st.Office.office13", backgroundColor: "#FF6600"
			state "getgd", label: 'INSTALL', action: "install", icon: "st.Office.office13", backgroundColor: "#FF0000", nextState: "wait"
			state "wait", label: 'WAIT', action: "ResetTiles", icon: "st.Office.office13", backgroundColor: "#FF0000"
		}
		valueTile("lastTriggered", "device.lastTriggered", width: 5, height: 1, decoration: "flat") {
			state("default", label: 'Taken on:\n${currentValue}', backgroundColor:"#ffffff")
		}
        standardTile("take", "device.image", width: 1, height: 1, canChangeIcon: false, inactiveLabel: true, canChangeBackground: false) {
            state "take", label: "Take", action: "on", icon: "st.camera.camera", backgroundColor: "#FFFFFF", nextState:"taking"
            state "taking", label:'Taking', action: "", icon: "st.camera.take-photo", backgroundColor: "#53a7c0"
            state "image", label: "Take", action: "Image Capture.on", icon: "st.camera.camera", backgroundColor: "#FFFFFF", nextState:"taking"
        }
        carouselTile("cameraDetails", "device.image", width: 6, height: 3) { }
		main "take"
		details(["testTriggered", "TestTrigger", "lastTriggered", "take", "cameraDetails"])
	}
}
def refresh() {
    poll()
}
def poll() {
	off()
}
def on() {
	def LocalDeviceBodyText = ''
	if (DeviceBodyText==null) { LocalDeviceBodyText = "GPIO=" } else { LocalDeviceBodyText = DeviceBodyText }

	if (UseJSON==true) {
		log.debug "GPIO=UseJSON= Triggered!!!"
		runCmd("GPIO=&UseJSON=")
	} else {
		log.debug "GPIO= Triggered!!!"
		runCmd("GPIO=")
	}
}
def off() {
	if (UseJSON==true) {
		log.debug "Test Triggered!!!"
		runCmd('Test=&UseJSON=', false)
	} else {
		log.debug "Test JSON Triggered!!!"
		runCmd('Test=', false)
	}
}
def install() {
	log.debug "INSTALL FUNCTION"
	sendEvent(name: "testTriggered", value: "TRY IN 3 MINUTES", unit: "", isStateChange: true)
	runCmd('GDInstall=', false)
}
def ResetTiles() {
	//RETURN BUTTONS TO CORRECT STATE
	sendEvent(name: "testswitch", value: "default", isStateChange: true)
	log.debug "Resetting tiles."
}
def runCmd(String varCommand, def useS3=true) {
	def host = DeviceIP
	def hosthex = convertIPtoHex(host).toUpperCase()
	def LocalDevicePort = ''
	if (DevicePort==null) { LocalDevicePort = "80" } else { LocalDevicePort = DevicePort }
	def porthex = convertPortToHex(LocalDevicePort).toUpperCase()
	device.deviceNetworkId = "$hosthex:$porthex"
	def userpassascii = "${HTTPUser}:${HTTPPassword}"
	def userpass = "Basic " + userpassascii.encodeAsBase64().toString()

	log.debug "The device id configured is: $device.deviceNetworkId"

	def path = DevicePath
	log.debug "path is: $path"
	log.debug "Uses which method: $DevicePostGet"
	def body = varCommand 
	log.debug "body is: $body"

	def headers = [:] 
	headers.put("HOST", "$host:$LocalDevicePort")
	headers.put("Content-Type", "application/x-www-form-urlencoded")
	if (HTTPAuth) {
		headers.put("Authorization", userpass)
	}
	log.debug "The Header is $headers"
	def method = "POST"
	try {
		if (DevicePostGet.toUpperCase() == "GET") {
			method = "GET"
			}
		}
	catch (Exception e) {
		settings.DevicePostGet = "POST"
		log.debug e
		log.debug "You must not have set the preference for the DevicePOSTGET option"
	}
	log.debug "The method is $method"
	try {
		def hubAction = new physicalgraph.device.HubAction(
			method: method,
			path: path,
			body: body,
			headers: headers
			)
		hubAction.options = [outputMsgToS3:useS3]
		log.debug "useS3===" + useS3
		//log.debug hubAction
		if (useS3==false) {
			sendEvent(name: "testTriggered", value: "", unit: "", isStateChange: true)
			if (body == "GDInstall=") {
				log.debug "STARTING GD INSTALL"
				sendEvent(name: "testTriggered", value: "TRY IN 5 MINUTES", unit: "", isStateChange: true) 
			}
		}
		hubAction
	}
	catch (Exception e) {
		log.debug "Hit Exception $e on $hubAction"
	}
}

def parse(String description) {
	log.debug "Parsing '${description}'"
	def whichTile = ''
	def map = [:]
	def retResult = []
	def descMap = parseDescriptionAsMap(description)
	def jsonlist = [:]
	def bodyReturned = ' '
	def headersReturned = ' '
	if (descMap["body"] && descMap["headers"]) {
		bodyReturned = new String(descMap["body"].decodeBase64())
		headersReturned = new String(descMap["headers"].decodeBase64())
	}
	log.debug "BODY---" + bodyReturned
	//log.debug "HEADERS---" + headersReturned

	def LocalDeviceBodyText = ''
	if (DeviceBodyText==null) { LocalDeviceBodyText = "GPIO=" } else { LocalDeviceBodyText = DeviceBodyText }

	//Image
	if (descMap["bucket"] && descMap["key"]) {
		putImageInS3(descMap)
		def imageDate = new Date().format("yyyy-MM-dd h:mm:ss a", location.timeZone)
		sendEvent(name: "lastTriggered", value: imageDate, unit: "", isStateChange: true)
	} else if (descMap["body"]) {
		if (headersReturned.contains("application/json")) {
			def body = new String(descMap["body"].decodeBase64())
			def slurper = new JsonSlurper()
			jsonlist = slurper.parseText(body)
			jsonlist.put ("Date", new Date().format("yyyy-MM-dd h:mm:ss a", location.timeZone))
			log.debug "JSON parsing..."
		} else if (headersReturned.contains("text/html")) {
			jsonlist.put ("Date", new Date().format("yyyy-MM-dd h:mm:ss a", location.timeZone))
			def data=bodyReturned.eachLine { line ->
				//if (line.contains(LocalDeviceBodyText + 'Success')) { jsonlist.put (LocalDeviceBodyText.replace("=",""), "Success") }
				//if (line.contains(LocalDeviceBodyText + 'Failed : Authentication Required!')) { jsonlist.put (LocalDeviceBodyText.replace("=",""), "Authentication Required!") }
				if (line.contains('Test=Success')) { jsonlist.put ("Test", "Success") }
				if (line.contains('Test=Failed : Authentication Required!')) { jsonlist.put ("Test", "Authentication Required!") }
			}
			log.debug "HTML parsing..."
		}
	}

	//RESET THE DEVICE ID TO GENERIC/RANDOM NUMBER. THIS ALLOWS MULTIPLE DEVICES TO USE THE SAME ID/IP
	device.deviceNetworkId = "ID_WILL_BE_CHANGED_AT_RUNTIME_" + (Math.abs(new Random().nextInt()) % 99999 + 1)

	if (descMap["body"] && (headersReturned.contains("application/json") || headersReturned.contains("text/html"))) {
		if (jsonlist."php5-gd"==false) {
			sendEvent(name: "testTriggered", value: jsonlist."Date" + "\nphp5-gd Not Installed", unit: "", isStateChange: true)
			sendEvent(name: "testswitch", value: "getgd", unit: "", isStateChange: true)
		} else if (jsonlist."php5-gd"==true) {
			sendEvent(name: "testTriggered", value: jsonlist."Date" + "\nphp5-gd Installed", unit: "", isStateChange: true)
			sendEvent(name: "testswitch", value: "default", isStateChange: true)
			def result = createEvent(name: "testswitch", value: "default", isStateChange: true)
			return result
		}
		if (jsonlist."Test"=="Authentication Required!") {
			sendEvent(name: "testTriggered", value: "\nUse Authentication Credentials", unit: "")
			def result = createEvent(name: "testswitch", value: "default", isStateChange: true)
			return result
		}
		if (jsonlist."GPIO"=="Authentication Required!") {
			//sendEvent(name: "lastTriggered", value: "\nUse Authentication Credentials", unit: "")
		}
	}

	log.debug jsonlist
}

def putImageInS3(map) {
	log.debug "firing s3"
    def s3ObjectContent
    try {
        def imageBytes = getS3Object(map.bucket, map.key + ".jpg")
        if(imageBytes)
        {
            s3ObjectContent = imageBytes.getObjectContent()
            def bytes = new ByteArrayInputStream(s3ObjectContent.bytes)
            storeImage(getPictureName(), bytes)
        }
    }
    catch(Exception e) {
        log.error e
    }
	finally {
    //Explicitly close the stream
		if (s3ObjectContent) { s3ObjectContent.close() }
	}
}
private getPictureName() {
	def pictureUuid = java.util.UUID.randomUUID().toString().replaceAll('-', '')
    log.debug pictureUuid
    def picName = device.deviceNetworkId.replaceAll(':', '') + "_$pictureUuid" + ".jpg"
	return picName
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
