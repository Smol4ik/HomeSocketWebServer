import asyncio
import json

import websockets

class Server:
    def __init__(self, host, port):
        self.host = host
        self.port = port

        self.clients_list = {}
        self.modified_clients_list = 0

    async def message_forwarding(self, name, message):
        if name in self.clients_list:
            try:
                selected_websocket = self.clients_list[name]["websocket"]
                await selected_websocket.send(json.dumps(message))
            except Exception as e:
                print(f"[ERROR]: {e}")
        else:
            print(f"Selected client {name} not connected")
            return

    async def active_client_list(self):
        count_client = len(self.clients_list)
        if count_client != self.modified_clients_list:
            self.modified_clients_list = count_client
            if "frontend" in self.clients_list:
                frontend_websocket = self.clients_list["frontend"]["websocket"]
                client_list_msg = {
                    "type": "update_user_list",
                    "active_clients": list(self.clients_list.keys())
                }
                await frontend_websocket.send(json.dumps(client_list_msg))
    async def websocket_connection(self, websocket):
        print(f"[CONNECTED] WebSocket connection established")
        name = None
        try:
            client_data = await websocket.recv()
            try:
                client_name = json.loads(client_data)
                if client_name and client_name.get("type") == "name":
                    name = client_name.get("name")
                    self.clients_list[name] = {"websocket": websocket}
                    print(f"[REGISTERED] Client: {name} has connected")
                    response = {"status": "success", "message": f"Welcome to server {name}"}
                    await websocket.send(json.dumps(response))
                    await self.active_client_list()
                else:
                    raise ValueError("Message without key 'name'")
            except (ValueError, json.JSONDecodeError):
                error_response = {"status": "error", "message": "Invalid message format"}
                await websocket.send(json.dumps(error_response))
                await websocket.close()
                return

            async for message in websocket:
                try:
                    data = json.loads(message)
                    if data and data.get("type") == "data_information":
                        data_sensor = data.get("data")
                        if "temperature" in data_sensor and "humidity" in data_sensor:
                            selected_client = data.get("selected_client")
                            msg_to_frontend = {"type": "data_information",
                                               "temperature": data_sensor["temperature"],
                                              "humidity": data_sensor["humidity"]}
                            await self.message_forwarding(selected_client, msg_to_frontend)
                        elif "led_state" in data_sensor:
                            selected_client = data.get("selected_client")
                            msg_to_esp = {"led_state": data_sensor["led_state"]}
                            await self.message_forwarding(selected_client, msg_to_esp)
                    else:
                        print(f"Unknown message type: {data.get("type")}")
                        break

                except Exception as e:
                    print(f"[ERROR]: {e}")
        except Exception as e:
            print("[DISCONNECTED] Client closed the connection")
        finally:
            if name and name in self.clients_list:
                del self.clients_list[name]
                print(f"[REMOVED] Client {name} removed from the list ")
                await self.active_client_list()

    async def start(self):
        print(f"[SERVER STARTED] ws://{self.host}:{self.port}")
        async with websockets.serve(self.websocket_connection, self.host, self.port):
            await asyncio.Future()

if __name__ == "__main__":
    host = '192.168.0.103'
    port = 8765
    server = Server(host, port)
    try:
        asyncio.run(server.start())
    except KeyboardInterrupt:
        print("[SERVER STOP!!]")