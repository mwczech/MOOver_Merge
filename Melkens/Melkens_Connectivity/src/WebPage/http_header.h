char html[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Roboto:wght@500&display=swap');
        @import url('https://fonts.googleapis.com/css2?family=Roboto:wght@300;500&display=swap');

        .arrows {
            font-size: 40px;
            color: red;
        }
        td.button {
            background-color: black;
            border-radius: 25%;
            box-shadow: 5px 5px #888888;
            text-align: center;
        }
        td.button:active {
            transform: translate(5px, 5px);
            box-shadow: none;
        }
        .dark-green {
            background-color: darkgreen !important;
        }
        .noselect {
            -webkit-touch-callout: none; /* iOS Safari */
            -webkit-user-select: none; /* Safari */
            -khtml-user-select: none; /* Konqueror HTML */
            -moz-user-select: none; /* Firefox */
            -ms-user-select: none; /* Internet Explorer/Edge */
            user-select: none; /* Non-prefixed version, currently supported by Chrome and Opera */
        }
        .slidecontainer {
            width: 100%;
            display: flex;
            flex-direction: row;
            justify-content: space-around;
            align-items: center;
        }
        .slider {
            width: 100%;
            height: 20px;
            border-radius: 5px;
            background: #d3d3d3;
            outline: none;
            opacity: 0.7;
            transition: opacity .2s;
        }
        .slider:hover {
            opacity: 1;
        }
        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 40px;
            height: 40px;
            border-radius: 50%;
            background: red;
            cursor: pointer;
        }
        .slider::-moz-range-thumb {
            width: 40px;
            height: 40px;
            border-radius: 50%;
            background: red;
            cursor: pointer;
        }
        body {
            background-color: white;
            display: flex;
            justify-content: center;
            background: rgb(174,226,255);
            background: linear-gradient(0deg, rgba(174,226,255,1) 0%, rgba(205,255,243,1) 35%, rgba(214,248,255,1) 100%);
            height: 100%;
            margin: 0;
            background-repeat: no-repeat;
            background-attachment: fixed;
        }
        .header_medium {
            font-family: 'Roboto', sans-serif;
        }
        p, ul, .speed {
            font-family: 'Roboto', sans-serif;
        }
        li {
            padding-top: 10px;
            padding-bottom: 10px;
        }
        .swider {
            width: 100%;
            align-items: center;
            margin-top: 20px;
        }
        .speed {
            margin-bottom: 10px;
        }
        .altimg {
            width: 100%;
            max-width: 1080px;
            height: auto;
            background-color: #03102b;
            margin-top: 150px;
            margin-bottom: 20px;
        }
        .header {
            background-color: #1f366a;
            display: flex;
            width: 100%;
            position: absolute;
            top: 0;
            left: 0;
            align-items: center;
            padding: 10px;
        }
        .logo {
            margin-right: 10px;
        }
        .header_big {
            font-family: 'Roboto', sans-serif;
            color: #fff;
            font-size: 20pt;
        }
        .arrows {
            color: #fff;
            background-color: #03102b;
        }
        td.button {
            box-shadow: 0px 5px 5px #4866ab;
            background-color: rgb(2, 10, 31);
            font-family: 'Roboto', sans-serif;
            font-size: 10px;
        }
        .send {
            margin-bottom: 50px;
        }
        .auger, .route, .route_step, .safety, .power, .charger, .IMU_recalibration{
            display: flex;
            background: #fff;
            height: auto;
            width: 100%;
            max-width: 400px;
            align-items: center;
            justify-content: space-evenly;
            border-radius: 5px;
            padding: 5px;
            box-shadow: 5px 5px 30px rgba(0,0,0,0.2);
            margin: 30px 0;
            font-family: system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;
            border-radius: 10px;
        }
        .buttons {
            display: grid;
            justify-content: center;
        }
        .control_buttons {
            display: flex;
            flex-direction: column;
            justify-content: space-between;
            width: 100%;
        }
        .control_buttons input {
            width: 100%;
            height: 50px;
            font-size: 20px;
            margin: 5px 0;
        }
    </style>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
</head>
<body class="noselect">
    <div class="all">
        <div class="header"><img src="https://melkens.com/wp-content/uploads/2022/03/Melkens-logo-blue-Recovered-1-300x59.png" class="logo"> <h2 class="header_big">Moover remote control</h2> </div>
        <div class="altimg"><img src="http://192.168.1.188:8080/video" id="video" style="width: 100%; height: auto;"></div>
        <br>
        <table id="mainTable" style="width: 100%; max-width: 400px; margin: auto; table-layout: fixed" cellspacing="10">
            <tbody>
                <tr>
                    <td></td>
                        <td class="button" onmouseup='sendData("FO");' ontouchend='sendData("FO");'><span class="arrows">⇧</span></td>                    <td></td>
                </tr>
                <tr>
                    <td class="button" onmouseup='handleButtonPress(this, "LE");' ontouchend='handleButtonPress(this, "LE");'><span class="arrows">⇦</span></td>
                    <td class="button" onmouseup='handleButtonPress(this, "ST");' ontouchend='handleButtonPress(this, "ST");'><span class="arrows">STOP</span></td>
                    <td class="button" onmouseup='handleButtonPress(this, "RI");' ontouchend='handleButtonPress(this, "RI");'><span class="arrows">⇨</span></td>
                </tr>
                <tr>
                    <td></td>
                    <td class="button" onmouseup='handleButtonPress(this, "RE");' ontouchend='handleButtonPress(this, "RE");'><span class="arrows">⇩</span></td>
                    <td></td>
                </tr>
                <tr>
                    <td colspan="3">
                        <div class="slidecontainer">
                            <b class="speed">Speed: <span id="slider_value"></span></b>
                            <input type="range" min="1" max="255" value="150" class="slider" id="Speed" oninput="updateTextInput(this.value);" onmouseup='sendData("V"+this.value);' ontouchend='sendData("V"+this.value);'>
                        </div>
                    </td>
                </tr>
            </tbody>
        </table>
        <div class="buttons">
            <div class="auger">
                <h3>Auger</h3>
                <input type="radio" id="auger_on" name="on_off" onmouseup='sendData("A1");' ontouchend='sendData("A1");'>
                <label for="on">ON</label>
                <input type="radio" id="auger_off" name="on_off" onmouseup='handleButtonPress(this, "A0");' ontouchend='handleButtonPress(this, "A0");'>
                <label for="off">OFF</label>
            </div>
            <div class="route_step">
                <h3>Route step selection</h3>
                <div class="control_buttons">
                    <input type="number" id="route_step_value" min="0" max="255" value="0" style="width: 80px; height: 30px; font-size: 20px;">
                    <input type="button" value="Set route step" onmouseup='handleButtonPress(this, "X" + document.getElementById("route_step_value").value);' ontouchend='handleButtonPress(this, "X" + document.getElementById("route_step_value").value);'>
                </div>
            </div>            
            <div class="route">
                <h3>Route</h3>
                <div class="control_buttons">
                    <input type="button" value="Play" onmouseup='handleButtonPress(this, "PL");' ontouchend='handleButtonPress(this, "PL");'>
                    <input type="button" value="Pause" onmouseup='handleButtonPress(this, "PA");' ontouchend='handleButtonPress(this, "PA");'>
                </div>
                <input type="radio" id="route_A" name="route" onmouseup='handleButtonPress(this, "TA");' ontouchend='handleButtonPress(this, "TA");'>
                <label for="A">A</label>
                <input type="radio" id="route_B" name="route" onmouseup='handleButtonPress(this, "TB");' ontouchend='handleButtonPress(this, "TB");'>
                <label for="B">B</label>
                <input type="radio" id="route_C" name="route" onmouseup='handleButtonPress(this, "TC");' ontouchend='handleButtonPress(this, "TC");'>
                <label for="C">C</label>
                <input type="radio" id="route_D" name="route" onmouseup='handleButtonPress(this, "TD");' ontouchend='handleButtonPress(this, "TD");'>
                <label for="D">D</label>
                <input type="radio" id="route_E" name="route" onmouseup='handleButtonPress(this, "TE");' ontouchend='handleButtonPress(this, "TE");'>
                <label for="E">E</label>
                <input type="radio" id="route_F" name="route" onmouseup='handleButtonPress(this, "TF");' ontouchend='handleButtonPress(this, "TF");'>
                <label for="F">F</label>
                <input type="radio" id="route_G" name="route" onmouseup='handleButtonPress(this, "TG");' ontouchend='handleButtonPress(this, "TG");'>
                <label for="G">G</label>
                <input type="radio" id="route_H" name="route" onmouseup='handleButtonPress(this, "TH");' ontouchend='handleButtonPress(this, "TH");'>
                <label for="H">H</label>
                <input type="radio" id="route_I" name="route" onmouseup='handleButtonPress(this, "TI");' ontouchend='handleButtonPress(this, "TI");'>
                <label for="I">I</label>
                <input type="radio" id="route_J" name="route" onmouseup='handleButtonPress(this, "TJ");' ontouchend='handleButtonPress(this, "TJ");'>
                <label for="J">J</label>
                <input type="radio" id="route_K" name="route" onmouseup='handleButtonPress(this, "TK");' ontouchend='handleButtonPress(this, "TK");'>
                <label for="K">K</label>
                
            </div>
            <div class="power">
                <h3>Power</h3>
                <input type="radio" id="power_on" name="power" onmouseup='handleButtonPress(this, "P1");' ontouchend='handleButtonPress(this, "P1");'>
                <label for="on">ON</label>
                <input type="radio" id="power_off" name="power" onmouseup='handleButtonPress(this, "P0");' ontouchend='handleButtonPress(this, "P0");'>
                <label for="off">OFF</label>
            </div>
            <div class="charger">
                <h3>Charging</h3>
                <input type="radio" id="charger_on" name="charger" onmouseup='handleButtonPress(this, "WH");' ontouchend='handleButtonPress(this, "WH");'>
                <label for="on">ON</label>
                <input type="radio" id="charger_off" name="charger" onmouseup='handleButtonPress(this, "WL");' ontouchend='handleButtonPress(this, "WL");'>
                <label for="off">OFF</label>
            </div>
            <div class="IMU_recalibration">
                <div class="control_buttons">
                    <input type="button" value="IMU Recalibration" onmouseup='handleButtonPress(this, "CL");' ontouchend='handleButtonPress(this, "CL");'>
                </div>
            </div>
            <div class="send">
                <h3 class="header_medium">Programming</h3>
                <p>Upload program to powerboard. Choose file and click program.</p>
                <input type="file" name="file" placeholder="choose file">
                <input type="button" id="program" name="program" value="Program" onmouseup='handleButtonPress(this, "PR");' ontouchend='handleButtonPress(this, "PR");'>
            </div>
        </div>
    </div>
    <script>
        function updateTextInput(val) {
            document.getElementById('slider_value').innerHTML = val; 
        }
        function handleButtonPress(element, motorData) {
            element.classList.add("dark-green");
            sendData(motorData);
            setTimeout(function() {
                element.classList.remove("dark-green");
            }, 200);
        }
        function sendData(motorData) {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                }
            };
            xhttp.open("GET", "setMotors?motorState=" + motorData, true);
            xhttp.send();
        }
        document.getElementById("mainTable").addEventListener("touchend", (e) => { event.preventDefault() });
    </script>
</body>
</html>
)rawliteral";
