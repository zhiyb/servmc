function colourd(s) {
  var bold = false, colour = null;
  s = s.replace(/\033\[([0-9;]+)m/g, function(esc, clr) {
    var res = clr.match(/[0-9]+/g);
    for (var i in res) {
      var c = parseInt(res[i], 10);
      if (c == 1)
        bold = true;
      else
        colour = c;
    }
    return "";
  });
  switch (colour) {
  case 30: colour = "#000000"; break;
  case 31: colour = "#bb0000"; break;
  case 32: colour = "#00bb00"; break;
  case 33: colour = "#bbbb00"; break;
  case 34: colour = "#0000bb"; break;
  case 35: colour = "#bb00bb"; break;
  case 36: colour = "#00bbbb"; break;
  case 37: colour = "#bbbbbb"; break;
  case 90: colour = "#555555"; break;
  case 91: colour = "#ff5555"; break;
  case 92: colour = "#55ff55"; break;
  case 93: colour = "#ffff55"; break;
  case 94: colour = "#5555ff"; break;
  case 95: colour = "#ff55ff"; break;
  case 96: colour = "#55ffff"; break;
  case 97: colour = "#ffffff"; break;
  default: colour = null; break;
  }
  var e = document.createElement("span");
  e.innerText = s;
  if (bold === true)
    e.style["font-weight"] = "bold";
  if (colour !== null)
    e.style["color"] = colour;
  return e.outerHTML;
}

var msgs = document.getElementById("msgs");
function append(s) {
  msgs.insertAdjacentHTML("beforeend", colourd(s).replace(/\n/g, "<br>"));
  msgs.scrollTop = msgs.scrollHeight;
}

var ws = new WebSocket("ws://" + document.domain + ":" + location.port, "web-console");
ws.onopen = function() {append("WebSocket connected\n");}
ws.onclose = function() {append("WebSocket disconnected\n");}
ws.onmessage = function(msg) {append(msg.data);}

document.getElementById("input").addEventListener('submit', function(e) {
  e.preventDefault();
  ws.send(document.forms["input"]["cmd"].value);
  document.forms["input"]["cmd"].value = "";
});
