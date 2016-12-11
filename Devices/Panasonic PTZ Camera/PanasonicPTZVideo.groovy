/**
 *  Panasonic Camera
 *
 *  Source code can be found here: https://github.com/JZ-SmartThings/SmartThings/blob/master/Devices/Panasonic%20PTZ%20Camera/PanasonicPTZCamera.groovy
 *
 *  Copyright 2016 JZ
 *
 *  Tested with Panasonic BL-C30A
 *  Thanks to: patrickstuart & blebson
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

metadata {
	definition (name: "Panasonic Camera", author: "JZ", namespace: "JZ") {
		capability "Image Capture"
		capability "Sensor"
		capability "Actuator"
        
		attribute "hubactionMode", "string"
        
		command "left"
		command "right"
		command "up"
		command "down"
        command "home"
        command "preset1"
        command "preset2"
        command "preset3"
        command "start"
		command "stop"
        command "vidOn"
        command "vidOff"
	}

    preferences {
    input("CameraIP", "string", title:"Camera IP Address", description: "Please enter your camera's IP Address", required: true, displayDuringSetup: true)
    input("CameraPort", "string", title:"Camera Port", description: "Re-type camera port, usually 80", defaultValue: 80 , required: true, displayDuringSetup: true)
    input("CameraAuth", "bool", title:"Does Camera require User Auth?", description: "Choose if the camera requires basic authentication", defaultValue: true, displayDuringSetup: true)
    input("CameraPostGet", "string", title:"Does Camera use a Post or Get, normally Get?", description: "Re-type GET", defaultValue: "GET", displayDuringSetup: true)
    input("CameraUser", "string", title:"Camera User", description: "Please enter your camera's username", required: false, displayDuringSetup: true)
    input("CameraPassword", "string", title:"Camera Password", description: "Please enter your camera's password", required: false, displayDuringSetup: true)
    }	

	mappings { path("/getInHomeURL") { action: [GET: "getInHomeURL"]} }
	
	simulator {
	}

    tiles (scale: 2){
		multiAttributeTile(name: "videoPlayer", type: "videoPlayer", width: 6, height: 4) {
			tileAttribute("device.switch2", key: "CAMERA_STATUS") {
				attributeState("on", label: "Active", icon: "st.camera.dlink-indoor", action: "vidOff", backgroundColor: "#79b821", defaultState: true)
				attributeState("off", label: "Inactive", icon: "st.camera.dlink-indoor", action: "vidOn", backgroundColor: "#ffffff")
				attributeState("restarting", label: "Connecting", icon: "st.camera.dlink-indoor", backgroundColor: "#53a7c0")
				attributeState("unavailable", label: "Unavailable", icon: "st.camera.dlink-indoor", action: "refresh.refresh", backgroundColor: "#F22000")
			}

			tileAttribute("device.camera", key: "PRIMARY_CONTROL") {
				attributeState("on", label: "Active", icon: "st.camera.dlink-indoor", backgroundColor: "#79b821", defaultState: true)
				attributeState("off", label: "Inactive", icon: "st.camera.dlink-indoor", backgroundColor: "#ffffff")
				attributeState("restarting", label: "Connecting", icon: "st.camera.dlink-indoor", backgroundColor: "#53a7c0")
				attributeState("unavailable", label: "Unavailable", icon: "st.camera.dlink-indoor", backgroundColor: "#F22000")
			}

			tileAttribute("device.startLive", key: "START_LIVE") {
				attributeState("live", action: "start", defaultState: true)
			}

			tileAttribute("device.stream", key: "STREAM_URL") {
				attributeState("activeURL", defaultState: true)
			}
			tileAttribute("device.betaLogo", key: "BETA_LOGO") {
				attributeState("betaLogo", label: "", value: "", defaultState: true)
			}
            }
        standardTile("take", "device.image", width: 1, height: 1, canChangeIcon: false, inactiveLabel: true, canChangeBackground: false) {
            state "take", label: "Take", action: "Image Capture.take", icon: "st.camera.camera", backgroundColor: "#FFFFFF", nextState:"taking"
            state "taking", label:'Taking', action: "", icon: "st.camera.take-photo", backgroundColor: "#53a7c0"
            state "image", label: "Take", action: "Image Capture.take", icon: "st.camera.camera", backgroundColor: "#FFFFFF", nextState:"taking"
        }
		standardTile("home", "device.image", width: 1, height: 1, canChangeIcon: false,  canChangeBackground: false, decoration: "flat") {
			state "home", label: "Home", action: "home", icon: "st.Home.home2"
		}
        standardTile("preset1", "device.image", width: 1, height: 1, canChangeIcon: false,  canChangeBackground: false, decoration: "flat") {
			state "preset1", label: "Preset 1", action: "preset1", icon: "st.camera.dlink-hdpan"
		}
        carouselTile("cameraDetails", "device.image", width: 3, height: 2) { }

        standardTile("blank", "device.image", width: 1, height: 1, canChangeIcon: false, canChangeBackground: false, decoration: "flat") {
            state "blank", label: "", action: "", icon: "", backgroundColor: "#FFFFFF"
        }
        standardTile("up", "device.image", width: 1, height: 1, canChangeIcon: false,  canChangeBackground: false, decoration: "flat") {
			state "up", label: "", action: "up", icon: "st.samsung.da.oven_ic_up"
		}
        standardTile("preset2", "device.image", width: 1, height: 1, canChangeIcon: false,  canChangeBackground: false, decoration: "flat") {
			state "preset2", label: "Preset 2", action: "preset2", icon: "st.camera.dlink-hdpan"
		}
        standardTile("preset3", "device.image", width: 1, height: 1, canChangeIcon: false,  canChangeBackground: false, decoration: "flat") {
			state "preset3", label: "Preset 3", action: "preset3", icon: "st.camera.dlink-hdpan"
		}
         standardTile("left", "device.image", width: 1, height: 1, canChangeIcon: false,  canChangeBackground: false, decoration: "flat") {
			state "left", label: "", action: "left", icon: "st.samsung.da.RAC_4line_01_ic_left"
		}
         standardTile("down", "device.image", width: 1, height: 1, canChangeIcon: false,  canChangeBackground: false, decoration: "flat") {
			state "down", label: "", action: "down", icon: "st.samsung.da.oven_ic_down"
		}
          standardTile("right", "device.image", width: 1, height: 1, canChangeIcon: false,  canChangeBackground: false, decoration: "flat") {
			state "right", label: "", action: "right", icon: "st.samsung.da.RAC_4line_03_ic_right"
		}
        main "take"
        details([ "videoPlayer", "take", "home", "preset1", "cameraDetails", "preset2", "up", "preset3", "left", "down", "right"])
    }
}



def start() {
	log.trace "start()"
	def dataLiveVideo = [
		OutHomeURL  : "http://${CameraUser}:${CameraPassword}@${CameraIP}:${CameraPort}/nphMotionJpeg?Resolution=320x240&Quality=Standard",
		InHomeURL   : "http://${CameraUser}:${CameraPassword}@${CameraIP}:${CameraPort}/nphMotionJpeg?Resolution=320x240&Quality=Standard",
		ThumbnailURL: "http://cdn.device-icons.smartthings.com/camera/dlink-indoor@2x.png",
		cookie      : [key: "key", value: "value"]
	]

	def event = [
		name           : "stream",
		value          : groovy.json.JsonOutput.toJson(dataLiveVideo).toString(),
		data		   : groovy.json.JsonOutput.toJson(dataLiveVideo),
		descriptionText: "Starting the livestream",
		eventType      : "VIDEO",
		displayed      : false,
		isStateChange  : true
	]
	sendEvent(event)
}

def stop() {
	log.trace "stop()"
}
def vidOn() {
	log.trace "on()"
	// no-op
}
def vidOff() {
	log.trace "off()"
	// no-op
}
def getInHomeURL() {
   [InHomeURL: "http://${CameraUser}:${CameraPassword}@${CameraIP}:${CameraPort}/nphMotionJpeg?Resolution=320x240&Quality=Standard"]
}

def take() {
	log.debug "Taking picture"
	cameraCmd("/SnapshotJPEG?Resolution=320x240")
}
def home() {
	log.debug "Moving to Home position"
    cameraCmd("/nphControlCamera?Direction=HomePosition")
}
def preset1() {
	log.debug "Moving to Preset 1 position"
    cameraCmd("nphControlCamera?Direction=Preset&PresetOperation=Move&Data=1")
}
def preset2() {
	log.debug "Moving to Preset 2 position"
    cameraCmd("nphControlCamera?Direction=Preset&PresetOperation=Move&Data=2")
}
def preset3() {
	log.debug "Moving to Preset 3 position"
    cameraCmd("nphControlCamera?Direction=Preset&PresetOperation=Move&Data=3")
}
def up() {
	log.debug "Tilt Up"
    cameraCmd("/nphControlCamera?Direction=TiltUp")
}
def left() {
	log.debug "Pan Left"
    cameraCmd("/nphControlCamera?Direction=PanLeft")
}
def right() {
	log.debug "Pan Right"
    cameraCmd("/nphControlCamera?Direction=PanRight")
}
def down() {
	log.debug "Tilt Down"
    cameraCmd("/nphControlCamera?Direction=TiltDown")
}


def cameraCmd(String varCommand) {
	def userpassascii = "${CameraUser}:${CameraPassword}"
	def userpass = "Basic " + userpassascii.encodeAsBase64().toString()
    def host = CameraIP 
    def hosthex = convertIPtoHex(host).toUpperCase()
    def porthex = convertPortToHex(CameraPort).toUpperCase()
    device.deviceNetworkId = "$hosthex:$porthex" 
    
    log.debug "The device id configured is: $device.deviceNetworkId"
    
    def path = varCommand 
    log.debug "path is: $path"
    log.debug "Requires Auth: $CameraAuth"
    log.debug "Uses which method: $CameraPostGet"
    
    def headers = [:] 
    headers.put("HOST", "$host:$CameraPort")
   	if (CameraAuth) {
        headers.put("Authorization", userpass)
    }
    
    log.debug "The Header is $headers"
    
    def method = "GET"
    try {
    	if (CameraPostGet.toUpperCase() == "POST") {
        	method = "POST"
        	}
        }
    catch (Exception e) {
    	settings.CameraPostGet = "GET"
        log.debug e
        log.debug "You must not of set the preference for the CameraPOSTGET option"
    }
    log.debug "The method is $method"
    try {
		def hubAction = new physicalgraph.device.HubAction(
			method: method,
			path: path,
			headers: headers
		)
		hubAction.options = [outputMsgToS3:true]
		log.debug hubAction
		hubAction
    }
    catch (Exception e) {
    	log.debug "Hit Exception $e on $hubAction"
    }
}

def parse(String description) {
    log.debug "Parsing '${description}'"
    def map = [:]
	def retResult = []
	def descMap = parseDescriptionAsMap(description)
	//Image
	if (descMap["bucket"] && descMap["key"]) {
		putImageInS3(descMap)
	}
    else if (descMap["headers"] && descMap["body"]){
    	def body = new String(descMap["body"].decodeBase64())
        log.debug "Body: ${body}"
	}
	else {
		log.debug "PARSE FAILED FOR: '${description}'"
    }
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

def parseDescriptionAsMap(description) {
	description.split(",").inject([:]) { map, param ->
		def nameAndValue = param.split(":")
		map += [(nameAndValue[0].trim()):nameAndValue[1].trim()]
	}
}

private getPictureName() {
	def pictureUuid = java.util.UUID.randomUUID().toString().replaceAll('-', '')
    log.debug pictureUuid
    def picName = device.deviceNetworkId.replaceAll(':', '') + "_$pictureUuid" + ".jpg"
	return picName
}

private String convertIPtoHex(ipAddress) { 
    String hex = ipAddress.tokenize( '.' ).collect {  String.format( '%02x', it.toInteger() ) }.join()
    log.debug "IP address entered is $ipAddress and the converted hex code is $hex"
    return hex
}

private String convertPortToHex(port) {
	String hexport = port.toString().format( '%04x', port.toInteger() )
    log.debug hexport
    return hexport
}

private Integer convertHexToInt(hex) {
	Integer.parseInt(hex,16)
}


private String convertHexToIP(hex) {
	log.debug("Convert hex to ip: $hex") 
	[convertHexToInt(hex[0..1]),convertHexToInt(hex[2..3]),convertHexToInt(hex[4..5]),convertHexToInt(hex[6..7])].join(".")
}

private getHostAddress() {
	def parts = device.deviceNetworkId.split(":")
    log.debug device.deviceNetworkId
	def ip = convertHexToIP(parts[0])
	def port = convertHexToInt(parts[1])
	return ip + ":" + port
}
