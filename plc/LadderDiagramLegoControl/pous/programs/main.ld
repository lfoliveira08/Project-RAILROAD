PROGRAM main
VAR
	M_EStop_Latch : bool;
	M_GarageFull : bool;
	M_Crossing_Occ : bool;
	Failsafe_Cond : bool;
	Failsafe_Prev : bool;
	Failsafe_Trip : bool;
	EndL_Stable : bool;
	EndR_Stable : bool;
	Xing_Latched : bool;
	Xing_EnteredA : bool;
	Xing_EnteredB : bool;
	GarageL_Deb : TON;
	GarageR_Deb : TON;
	Xing_Clear_Delay : TOF;
	TRIP_CM : int := 5;
	Power_FB : bool AT %IX100.0;
	EStop_Button : bool AT %IX100.1;
	VR_OK : bool AT %IX100.2;
	IR_CrossA : bool AT %IX100.3;
	IR_CrossB : bool AT %IX100.4;
	IR_EndL : bool AT %IX100.5;
	IR_EndR : bool AT %IX100.6;
	Ultrasonic_Dist : int AT %IW103;
	Track_Power_Cmd : bool AT %QX100.0;
	Buzzer_Cmd : bool AT %QX100.1;
	J1_Angle : int AT %QW100;
	J2_Angle : int AT %QW101;
	Crossing_Gate : int AT %QW102;
	LED_Mode : int AT %QW103;
	Failsafe_Servo : int AT %QW104;
	LCD1_Code : int AT %QW105;
	LCD2_Code : int AT %QW106;
	LCD3_Code : int AT %QW107;
	Reset_Cmd : bool AT %QX80.0;
	Run_Cmd : bool AT %QX80.1;
	EStop_SW : bool AT %QX80.2;
	Manual_Mode : bool AT %QX80.3;
	J1_Cmd : bool AT %QX80.4;
	J2_Cmd : bool AT %QX80.5;
END_VAR

{
  "name": "main",
  "rungs": [
    {
      "id": "rung_main_782e97cc-ca78-4387-814d-acfd936cc994",
      "comment": "",
      "defaultBounds": [
        300,
        100
      ],
      "reactFlowViewport": [
        303,
        120
      ],
      "nodes": [
        {
          "id": "left-rail-rung_main_782e97cc-ca78-4387-814d-acfd936cc994",
          "type": "powerRail",
          "position": {
            "x": 0,
            "y": 30
          },
          "height": 40,
          "width": 3,
          "measured": {
            "width": 3,
            "height": 40
          },
          "draggable": false,
          "selectable": false,
          "data": {
            "handles": [
              {
                "glbPosition": {
                  "x": 3,
                  "y": 50
                },
                "relPosition": {
                  "x": 3,
                  "y": 20
                },
                "id": "left-rail",
                "position": "right",
                "type": "source",
                "isConnectable": false,
                "style": {
                  "top": 20,
                  "right": -3
                }
              }
            ],
            "inputHandles": [],
            "outputHandles": [
              {
                "glbPosition": {
                  "x": 3,
                  "y": 50
                },
                "relPosition": {
                  "x": 3,
                  "y": 20
                },
                "id": "left-rail",
                "position": "right",
                "type": "source",
                "isConnectable": false,
                "style": {
                  "top": 20,
                  "right": -3
                }
              }
            ],
            "outputConnector": {
              "glbPosition": {
                "x": 3,
                "y": 50
              },
              "relPosition": {
                "x": 3,
                "y": 20
              },
              "id": "left-rail",
              "position": "right",
              "type": "source",
              "isConnectable": false,
              "style": {
                "top": 20,
                "right": -3
              }
            },
            "numericId": "9953575",
            "variant": "left",
            "variable": {
              "name": ""
            },
            "executionOrder": 0,
            "draggable": false,
            "selectable": false,
            "deletable": false
          }
        },
        {
          "id": "right-rail-rung_main_782e97cc-ca78-4387-814d-acfd936cc994",
          "type": "powerRail",
          "position": {
            "x": 300,
            "y": 30
          },
          "height": 40,
          "width": 3,
          "measured": {
            "width": 3,
            "height": 40
          },
          "draggable": false,
          "selectable": false,
          "data": {
            "handles": [
              {
                "glbPosition": {
                  "x": 297,
                  "y": 50
                },
                "relPosition": {
                  "x": 3,
                  "y": 20
                },
                "id": "right-rail",
                "position": "left",
                "type": "target",
                "isConnectable": false,
                "style": {
                  "top": 20,
                  "left": -3
                }
              }
            ],
            "inputHandles": [
              {
                "glbPosition": {
                  "x": 297,
                  "y": 50
                },
                "relPosition": {
                  "x": 3,
                  "y": 20
                },
                "id": "right-rail",
                "position": "left",
                "type": "target",
                "isConnectable": false,
                "style": {
                  "top": 20,
                  "left": -3
                }
              }
            ],
            "outputHandles": [],
            "inputConnector": {
              "glbPosition": {
                "x": 297,
                "y": 50
              },
              "relPosition": {
                "x": 3,
                "y": 20
              },
              "id": "right-rail",
              "position": "left",
              "type": "target",
              "isConnectable": false,
              "style": {
                "top": 20,
                "left": -3
              }
            },
            "numericId": "4500570",
            "variant": "right",
            "variable": {
              "name": ""
            },
            "executionOrder": 0,
            "draggable": false,
            "selectable": false,
            "deletable": false
          }
        }
      ],
      "edges": [
        {
          "id": "e_left-rail-rung_main_782e97cc-ca78-4387-814d-acfd936cc994_right-rail-rung_main_782e97cc-ca78-4387-814d-acfd936cc994",
          "source": "left-rail-rung_main_782e97cc-ca78-4387-814d-acfd936cc994",
          "sourceHandle": "left-rail",
          "target": "right-rail-rung_main_782e97cc-ca78-4387-814d-acfd936cc994",
          "targetHandle": "right-rail",
          "type": "smoothstep"
        }
      ]
    }
  ]
}
END_PROGRAM