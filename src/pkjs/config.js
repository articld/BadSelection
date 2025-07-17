module.exports = [
  {
    "type": "heading",
    "defaultValue": "Watchface Configuration"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Clock Settings"
      },

      //----------------- 
      {
        "type": "color",
        "messageKey": "AccentColor",
        "defaultValue": "0x0000AA",
        "label": "Accent Color"
      },

      //----------------- 
      {
        "type": "color",
        "messageKey": "TimeColor",
        "defaultValue": "0xFFFFFF",
        "label": "Time Color"
      },


      //----------------- 
      {
        "type":"select",
        "messageKey":"TimeBoxPosition",
        "label":"Time position",
        "defaultValue": 0,
        "options":[

          {
            "label":"Left",
            "value": 0
          },

          {
            "label":"Middle",
            "value": 1
          },

          {
            "label":"Right",
            "value": 2
          }

        ]
      }

    ]

  },

  {
    "type":"section",
    "items":[
      {
      "type":"heading",
      "defaultValue":"TextGrid Configuration"
      },

      //----------------- 
      {
        "type": "color",
        "messageKey": "TextGridColor",
        "defaultValue": "0x555555",
        "label": "Text Grid Color"
      },

      //----------------- 
      {
        "type": "color",
        "messageKey": "BGColor",
        "defaultValue": "0x000000",
        "label": "Background Color"
      },

      //----------------- 
      {
        "type":"toggle",
        "messageKey":"TextGridAnimation",
        "label": "Enable/Disable the TextGrid animation",
        "defaultValue":true
      }
    ]
  },

  {
    "type":"section",
    "items":[
      {
        "type":"heading",
        "defaultValue":"Date Settings"
      },

      {
        "type":"toggle",
        "messageKey":"Date",
        "label":"Show/Hide the current date",
        "defaultValue":true
      }
    ]
  },

  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];