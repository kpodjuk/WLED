<p align="center">
  <img src="/images/wled_logo_akemi.png">
</p>
  
# Welcome to my branch of WLED! 💡

## ⚙️ I have extanded WLED to include:
- Changing active preset based on PIR movement sensor.
- Sending IR commands to cheap RGB lightbulbs that normally are possible to control only with a IR remote. Now it's possible to control them from app/macro/based on movement thanks to IR diode connected to MCU. ([Bulbs like this]https://shopee.pl/LED-RGB-Bulb-E27-Light-RGB-RGBW-RGBWW-Dimmable-IR-Remote-10W-220V-15W-Magic-H1U8-AC-Club-W7H0-i.593680934.15619685353).
- Color approximation feature for IR bulbs. They have very limited amount of colors (16, while RGB strips have 16mln), this feature will try to find closest possible bulb color based on first LED of RGB strip.
- Additional web interface do manually change colors/brightness/effect of IR bulbs.

A lot of stuff in this repo is obviously done incorrectly 😖.



