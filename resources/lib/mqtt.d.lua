---@meta

---@class Mqtt
Mqtt = {}

---Create an MQTT instance.
---@param name portName Target port name.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return mqtt
function Mqtt.new(name, timeout) end

---@class mqttOptions
---@field protocol? "MQTT 3.1" | "MQTT 3.1.1" | "MQTT 5.0" (default: "MQTT 3.1")
---@field clientId string
---@field cleanStart? boolean (default: true) Whether the connection starts a new session or is a continuation of an existing session.
---@field willQoS nil WIP
---@field willRetain nil WIP
---@field username? string (default: "")
---@field password? password (default: "")
---@field keepAlive? integer (default: 60) Maximum time in **seconds** between messages to keep the connection alive.
---@field properties nil WIP

---@class mqtt
---@field options mqttOptions
mqtt = {}

---Send a CONNECT packet to the server.
function mqtt:connect() end