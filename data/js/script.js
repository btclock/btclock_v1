var screens = ["Block Height", "Moscow Time", "Ticker", "Time"];

getBcStatus = () => {
    fetch('/api/status', {
        method: 'get'
    })
        .then(response => response.json())
        .then(jsonData => {

            var source = document.getElementById("entry-template").innerHTML;
            var template = Handlebars.compile(source);
            var context = { currentScreen: jsonData.currentScreen, rendered: jsonData.rendered, data: jsonData.data, screens: screens };
            document.getElementById('output').innerHTML = template(context);
        })
        .catch(err => {
            //error block
        });
}

interval = setInterval(getBcStatus, 2500);
getBcStatus();

fetch('/api/settings', {
    method: 'get'
})
    .then(response => response.json())
    .then(jsonData => {
        var fgColor = ("0x" + jsonData.fgColor.toString(16).toUpperCase());
        if (jsonData.epdColors == 2) {
            document.getElementById('fgColor').querySelector('[value="0xF800"]').remove();
            document.getElementById('bgColor').querySelector('[value="0xF800"]').remove();
        }

        document.getElementById('output').classList.add("fg-" + jsonData.fgColor.toString(16));
        document.getElementById('output').classList.add("bg-" + jsonData.bgColor.toString(16));

        document.getElementById('fgColor').value = fgColor;
        document.getElementById('bgColor').value = "0x" + jsonData.bgColor.toString(16).toUpperCase();
    })
    .catch(err => {
        console.log('error', err);
    });


var settingsForm = document.querySelector('#settingsForm');
settingsForm.onsubmit = function (event) {
    var formData = new FormData(settingsForm);

    fetch("/api/settings",
        {
            body: formData,
            method: "post"
        }).then(() => {
            console.log('Submitted');
        });

    return false;
}

var textForm = document.querySelector('#customTextForm');
textForm.onsubmit = function (event) {
    var formData = new FormData(textForm);

    fetch('/api/show/text/' + encodeURIComponent(formData.get('customText')), {
        method: 'get'
    })
        .then(response => response.json())
        .catch(err => {
            //error block
        });
    return false;
}

changeScreen = (id) => {
    fetch('/api/show/screen/' + encodeURIComponent(id), {
        method: 'get'
    })
        .then(response => response.json())
        .catch(err => {
            //error block
        });
}