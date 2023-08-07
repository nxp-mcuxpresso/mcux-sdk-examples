var g_state = "ap";


function checkIfFirefox(){
  if(navigator.userAgent.toLowerCase().indexOf('firefox') > -1){
    open_error_dialogue("Firefox is not supported by this application. Use Microsoft Edge or Google Chrome.",
      "Browser not supported");
  }
  scan();
}


function showPassword() {
  var x = document.getElementById("post_password");
  if (x.type === "password") {
    x.type = "text";
  } else {
    x.type = "password";
  }
}

function populateWifiNetworks(networks) {
    var objectWN = document.getElementById("wifi_list");
    if (null == networks){
        open_error_dialogue("Something went wrong during the scan. Please try again or check the logs.", "Scan failed");
        return;
    }


    if(0 === networks.length) {
        objectWN.innerHTML  = "<div id='wifi_no_networks'>No Networks Available</div>";
        return;
    }

    // Mark 5Ghz networks
    networks.forEach((net)=>{
        if(net.channel > 13){
            net.is5GHz = true;
            // This is used so that during the duplication finding, we don't remove 5G networks with same name as 2.4G nets
            net.ssidWith5GHz = net.ssid + "5GHzNetworkLabel<Used to distinguish 2.4G and 5Ghz>";
        } else {
            net.is5GHz = false;
            net.ssidWith5GHz = net.ssid;
        }
    });

    // Sort Wifi Networks Alphabetically
    // Push all Enterprise and hidden networks to the bottom
    // If multiple stations with same SSID are found, sort them according to signal strength
    networks.sort((a, b)=>{

        // Sort Enterprise networks to the bottom
        let netA = /ENT/i.test(a.security);
        let netB = /ENT/i.test(b.security);

        if(netA === netB){
            netA = a.ssid.toUpperCase();
            netB = b.ssid.toUpperCase();

            // Sort Hidden Networks to the bottom
            if (netA === "" && netB !== ""){
                return 1;
            }
            if (netA !== "" && netB === ""){
                return -1;
            }

            // 5Ghz is different....
            netA += a.is5GHz ? "5GHzNetworkLabel<>" : "";
            netB += b.is5GHz ? "5GHzNetworkLabel<>" : "";

            // If same SSID, sort by signal strength
            if(netA === netB){
                netA = Math.abs(parseInt(a.signal.replace(/[a-z,A-Z]/g, '')));
                netB = Math.abs(parseInt(b.signal.replace(/[a-z,A-Z]/g, '')));
            }
        }
        return (netA < netB) ? -1 : (netA > netB) ? 1 : 0;

    })

    // Mark all duplicates
    networks.forEach((net, index, array)=>{
        if (index !== 0) {
            if(net.ssidWith5GHz == array[index-1].ssidWith5GHz){
                net.duplicate = true;
                return;
            }
        }
        net.duplicate = false;
    });



    let net_html = "";
    networks.forEach((net)=>{
        let otherClasses = '';
        let display5G = '';
        let open;

        if (net.security.trim() == "" || net.security.trim() == "Open"){
            open = true;
            net.security = "Open";
        } else {
            open = false;
            net.security = net.security.trim();
        }

        if (net.ssid == "") {
            net.ssid = "[Hidden SSID]";
            otherClasses += 'hidden_network ';
        }

        let enterprise = /ENT/i.test(net.security);

        if(enterprise) {
            otherClasses += 'enterprise_network ';
        }

        // Mark duplicate networks
        if(net.duplicate) {
            otherClasses += "duplicate_network ";
        }

        // Label 5Ghz Networks
        if(net.is5GHz){
            display5G = "[5GHz]";
            otherClasses += "wifi5GHz_network ";
        }

        net_html += `<div class="wifi_network ${otherClasses}" onclick='connect_net("${net.ssid}", ${open}, ${enterprise}, "${net.security}")'>
                     <span class="wifi_network_value wifi_network_ssid">${net.ssid} (${net.security}) ${display5G}</span>
                     <br>
                      <span  class="wifi_network_label">BSSID:</span>  <span class="wifi_network_value">${net.bssid}</span>
                    <br>
                    <span  class="wifi_network_label">Channel:</span>  <span class="wifi_network_value">${net.channel}</span>
                    <br>
                    <span class="wifi_network_label">Signal Strength:</span> <span class="wifi_network_value">${net.signal}</span>
                    <br>
                </div>`;

    });
    objectWN.innerHTML = net_html;
    renderDuplicates(); // Show/Hide duplicates

}

function populateBoardInfo(data){
    document.getElementById("boardName").innerHTML = data["name"];
    document.getElementById("boardIP").innerHTML = data["ip"];

    if(data.status == "client"){
        document.getElementById("statusAP").classList.remove("active")
        document.getElementById("statusCLI").classList.add("active")
        g_state = "client";
    } else {
        document.getElementById("statusCLI").classList.remove("active")
        document.getElementById("statusAP").classList.add("active")
        g_state = "ap";
    }

}

function renderDuplicates(){
    let nets = document.getElementsByClassName("duplicate_network");
    let style = ""

    // If the check box is checked, display duplicate stations
    if(document.getElementById("all_nets").checked){
        style = "block";
    } else {
        style = "none";
    }

    for(let i = 0; i < nets.length; i++){
        nets[i].style.display = style
    }
}

function connect_net(ssid, open, ent, security){
    if (ssid == "[Hidden SSID]"){
        open_success_dialogue("Connecting to hidden SSIDs is not supported.", "Connecting to Hidden SSID");
        return;
    }

    if(ent){
        open_success_dialogue("Connecting to Enterprise networks is not supported.", "Connecting to Enterprise Wi-Fi");
        return;
    }

    if (g_state == "client") {
        open_cJoin_dialogue();
        return;
    }


    document.getElementById("post_ssid").value = ssid;
    document.getElementById("post_security").value = security;
    document.getElementById("post_password").value = "";
    document.getElementById("post_errors").innerHTML = "";
    openDialogue("ap_password_page", true)

    if(open){
        document.getElementById("password_input_field").style.display = "none";

    } else {
        document.getElementById("password_input_field").style.display = "block";
    }

}



function validateForm() {
    if(document.getElementById("post_ssid").value.length == 0) {
        document.getElementById("post_errors").innerHTML = "SSID can't be blank.";
        return false;
    }

    // If we have an open access point, dont check password
    if(document.getElementById("password_input_field").style.display == "block") {
        if(document.getElementById("post_password").value.length == 0) {
            document.getElementById("post_errors").innerHTML = "Password can't be blank.";
            return false;
        }

        if(document.getElementById("post_password").value.length < 8) {
            document.getElementById("post_errors").innerHTML = "Password must be at least 8 characters.";
            return false;
        }
        if(document.getElementById("post_password").value.length > 63) {
            document.getElementById("post_errors").innerHTML = "Password can't have more than 63 characters.";
            return false;
        }

    }


    closeDialogue("ap_password_page");

    attemptConnect();

    return true;
}


function mkRequest(method, url, params, cb, timeout){
    var req = new XMLHttpRequest();
    req.open(method, url);
     var requestTimeout = setTimeout(()=>{
        cb(new Error("timeout"), null);
    }, timeout);

    req.onload = function(){
        clearTimeout(requestTimeout);
        cb(null, req.response);
    }
    req.onerror = function(){
        clearTimeout(requestTimeout);
        cb(req.response, null);
    }
    req.send(params);
}

function getBoardInfo(cb) {
    mkRequest("GET","status.cgi",null,(err, data_status)=>{
        cb(err, data_status);
    }, 1000);
}

function attemptConnect(){

    open_mode_switch_dialogue();

    var ssid =  document.getElementById("post_ssid").value;
    var password =  document.getElementById("post_password").value;
    var security =  document.getElementById("post_security").value;
    var params = `post_ssid=${ssid}&post_passphrase=${password}&post_security=${security}`

    mkRequest("POST", "post.cgi", params, (err, data)=>{
        if(err){
            if(err.message === "timeout"){
                open_error_dialogue(`Attempt to Join '${ssid}' timed out. The connection was likely interrupted during the network switch and the response was lost. This is not necessarily an error and the board probably successfully switched to the new AP. You may need to manually switch your device to the '${ssid}' AP in order to continue. Check the console for further instructions and the new board IP address.`, "Join request timed out");
            } else {
                open_error_dialogue(`Attempt to Join '${ssid}' encountered an error: ${err}`, "Join request encountered an error");

            }
        } else {
            try{
                var data = JSON.parse(data);
                if(data.status == "success"){
                    open_success_dialogue(`Now join the <b>${ssid}</b> network and browse to the IP: <a href="http://${data.new_ip}">${data.new_ip}</a>.`,"Successfully connected");
                } else {
                    open_error_dialogue(`Error connecting to ${ssid}. Please check the password and try again.`, "Wrong Password");
                }
            } catch (e) {
                if (/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent)){
                    open_success_dialogue("Please check board serial console output for new device IP", "Connection lost!");
                } else {
                    open_error_dialogue("Error while parsing JSON: " + e, "An Error has occurred");
                }

            }
        }

    }, 60000);
}



function scan(){
    openDialogue("loader_page");
    mkRequest("GET","get.cgi",null,(err, data_scan)=>{
        console.log(err, data_scan)
        if(err){
            if(err.message == "timeout"){
                open_error_dialogue("Scan has timed out. Try again later.", "Scan timed out");
            } else {
                open_error_dialogue("Error requesting scan:" + e, "An Error has occurred");
            }
        }
        else
        {
            getBoardInfo((err, data_status)=>{
                if(err){
                    if(err.message == "timeout"){
                        open_error_dialogue("Status has timed out. Try again later.", "Scan timed out");
                    } else {
                        open_error_dialogue("Error requesting status:" + e, "An Error has occurred");
                    }
                }

                setTimeout(()=>{
                    closeDialogue("loader_page");
                },300);
                closeDialogue("error_banner");
                try {
                    var json_scan = JSON.parse(data_scan);
                    var json_status = JSON.parse(data_status);
                    populateBoardInfo(json_status["info"]);
                    populateWifiNetworks(json_scan["networks"]);
                } catch (e) {
                    open_error_dialogue("Error while parsing JSON:" + e, "An Error has occurred");
                }
            });
        }
    }, 30000);

}

/* Make request to server */

function scanold()
{

    openDialogue("loader_page");

    var http = new XMLHttpRequest();

    var requestTimeout = setTimeout(()=>{
        open_error_dialogue("Scan has timed out. Try again later.", "Scan timed out")
    }, 30000);

    http.onreadystatechange = function() {
        if (http.readyState == 4)
        {
            if (http.status == 200)
            {
                setTimeout(()=>{
                    closeDialogue("loader_page")
                }, 300);
                clearTimeout(requestTimeout);
                closeDialogue("error_banner");
                try {
                    var data = JSON.parse(http.responseText);
                    populateBoardInfo(data["info"]);
                    populateWifiNetworks(data["networks"]);
                } catch (e) {
                    open_error_dialogue("Error while parsing JSON:" + e, "An Error has occurred");
                }


            }
            data_received = 0;
        }
    };


    http.open('GET', 'get.cgi', true);
    http.send(null);
}


function openDialogue(id, stopPropagation){
    document.getElementById(id).style.display = "block";

    if(typeof stopPropagation !== "undefined" && stopPropagation){
        document.getElementById(`${id}_dialogue`).onclick = function(event){
            event.stopPropagation();
        }
    }
}

function closeDialogue(id){
    document.getElementById(id).style.display = "none";
}


function open_mode_switch_dialogue(){
    openDialogue("mode_switch_loader");
    document.getElementById("mode_switch_network_name").innerHTML = document.getElementById("post_ssid").value;
}


function open_clear_board_dialogue() {
    openDialogue("clear_confirm", true);
}

function open_cJoin_dialogue(){
    openDialogue("clientJoin_banner", true);
}

function open_success_dialogue(msg, title){
    // Close all other dialogues
    closeDialogue("loader_page");
    closeDialogue("mode_switch_loader");
    closeDialogue("error_banner");
    closeDialogue("clear_confirm");
    closeDialogue("clientJoin_banner");


    document.getElementById("success_msg").innerHTML = msg;
    document.getElementById("success_title").innerHTML = title;
    openDialogue("success_banner");
}

function open_error_dialogue(msg, title){
    // Close all other dialogues
    closeDialogue("loader_page");
    closeDialogue("mode_switch_loader");
    closeDialogue("success_banner");
    closeDialogue("clear_confirm");
    closeDialogue("clientJoin_banner");

    document.getElementById("error_msg").innerHTML = msg;
    document.getElementById("error_title").innerHTML = title;
    openDialogue("error_banner");
}

function clear_config(){
    var http = new XMLHttpRequest();
    http.open("GET", "reset.cgi", true);
    http.setRequestHeader("Content-type","application/x-www-form-urlencoded");
    http.send(null);

    var connectionTimeout = setTimeout(()=>{
        open_error_dialogue("Attempt to reset timed out. The request was probably interrupted during the network switch. Try switching your device network back to the board AP. Check the console for further instructions.", "Request timed out")
    }, 5000);
    http.onload = function() {
        clearTimeout(connectionTimeout);
        console.log(http.responseText);



        try{
            var data = JSON.parse(http.responseText);
            if(data.status == "success"){
                open_success_dialogue(`Successfully cleared the flash memory and reset to an AP. Please connect you device back to the AP and browse to the IP: <a href="http://${data.new_ip}">${data.new_ip}</a>.`, "Success");

            } else {
                open_error_dialogue("Failed to clear the flash memory. Check the console for more info.", "An Error has occurred");
            }
        } catch (e) {
            open_error_dialogue("Error while parsing JSON:" + e, "An Error has occurred");

        }
    }

}
