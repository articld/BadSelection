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
        "defaultValue": "Colors"
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
        "messageKey": "BGColor",
        "defaultValue": "0x000000",
        "label": "Background Color"
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
        "type": "color",
        "messageKey": "TextGridColor",
        "defaultValue": "0x555555",
        "label": "Text Grid Color"
      },
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];