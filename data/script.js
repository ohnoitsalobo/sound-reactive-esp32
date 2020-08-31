var connection = new WebSocket('ws://'+location.hostname+'/ws',['arduino']);

function startSocket(){
    // connection = new WebSocket('ws://'+document.location.host+'/ws',['arduino']);
    connection.onopen = function () {
        connection.send('Connect ' + new Date());
    };
    connection.onerror = function (error) {
        console.log('WebSocket Error ', error);
    };
    connection.onclose = function(){
        console.log('WebSocket connection closed');
    };
}


function onBodyLoad(){
    startSocket();
    process();
}

var m = 0, r = 0, g = 0, b = 0, h = 0, s = 0, v = 0, _ll = false, _rr = false;
var rr, gg, bb;

function reset(){
    connection.send('reset');
    console.log('reset'); 
    location.reload();
}

function next(){
    connection.send('next');
    console.log('next'); 
}

function prev(){
    connection.send('prev');
    console.log('prev'); 
}

function sendRGB(_temp) {
    process();
    var rgb, rgbstr;
    if(_temp == "M") {
        rgb = m ;
        _ll = document.getElementById('ll').checked = true;
        _rr = document.getElementById('rr').checked = true;
    }
    if(_temp == "R") { rgb = r ; }
    if(_temp == "G") { rgb = g ; }
    if(_temp == "B") { rgb = b ; }
    if(_temp == "H") { rgb = h ; }
    if(_temp == "S") { rgb = s ; }
    if(_temp == "V") { rgb = v ; }
    rgbstr = _temp + rgb.toString(16);
    if(_ll && _rr) { rgbstr += 'B'; }
    else if(!_ll && !_rr) { rgbstr += 'X'; }
    else{
             if(_ll) { rgbstr += 'L'; }
        else if(_rr) { rgbstr += 'R'; }
    }
    console.log('RGB: ' + rgbstr); 
    connection.send(rgbstr);
    c = [r, g, b];
}

function process(){
    m =  document.getElementById('m' ).value;
    r =  document.getElementById('r' ).value; var _r = r * 100/255;
    g =  document.getElementById('g' ).value; var _g = g * 100/255;
    b =  document.getElementById('b' ).value; var _b = b * 100/255;
    h =  document.getElementById('h' ).value;
    s =  document.getElementById('s' ).value; var _s = s * 100/255;
    v =  document.getElementById('v' ).value; var _v = v * 100/255;
    _ll = document.getElementById('ll').checked;
    _rr = document.getElementById('rr').checked;
    
    var mode = "";
    if     (m==0) { mode = "Sound response"; }
    else if(m==1) { mode = "Patterns";       }
    else if(m==2) { mode = "Manual";         }
    
    document.getElementById('mval' ).innerHTML = mode;
    document.getElementById('rval' ).innerHTML = _r.toFixed(0) + '%' ;
    document.getElementById('gval' ).innerHTML = _g.toFixed(0) + '%' ;
    document.getElementById('bval' ).innerHTML = _b.toFixed(0) + '%' ;
    document.getElementById('hval' ).innerHTML = h ;
    document.getElementById('sval' ).innerHTML = _s.toFixed(0) + '%' ;
    document.getElementById('vval' ).innerHTML = _v.toFixed(0) + '%' ;
    
    var en = false;
    if(m != 2){
        en = true;
    }
    document.getElementById('r' ).disabled = en;
    document.getElementById('g' ).disabled = en;
    document.getElementById('b' ).disabled = en;
    document.getElementById('h' ).disabled = en;
    document.getElementById('s' ).disabled = en;
}

let img;
function setup(){
    let _size = 0.9*((windowWidth < windowHeight) ? windowWidth : windowHeight);
    let cnv = createCanvas(_size, _size);
    cnv.parent('p5js');
    cnv.style('border-radius: 10%;');
    frameRate(10);
    img = loadImage('hue_circle.png');
}

function rainbowHeader(){
    let _hue = millis()/50;
    document.getElementById('header').style['background'] = 
        "linear-gradient(     to right, hsl(" + 
         _hue     %360 + ", 100%, 50%), hsl(" + 
        (_hue+60 )%360 + ", 100%, 50%), hsl(" + 
        (_hue+120)%360 + ", 100%, 50%), hsl(" + 
        (_hue+180)%360 + ", 100%, 50%), hsl(" + 
        (_hue+240)%360 + ", 100%, 50%), hsl(" + 
        (_hue+300)%360 + ", 100%, 50%))";
    document.getElementById('header').style['color'] = "transparent";
    document.getElementById('header').style['-webkit-background-clip'] = "text";
}

function draw(){
    rainbowHeader();
    colorWheel();
}

let c = [0, 0, 0];
let leftcolor, rightcolor;
function colorWheel(){
    if (
    m == 2           &&
    mouseIsPressed   &&
    mouseX <= width  && mouseX >= 0 &&
    mouseY <= height && mouseY >= 0 ){
        colorMode(RGB);
        c = get(mouseX, mouseY);
        document.getElementById('r' ).value = c[0];
        document.getElementById('g' ).value = c[1];
        document.getElementById('b' ).value = c[2];
        document.getElementById('rval' ).innerHTML = (c[0]/255*100).toFixed(0) + '%';
        document.getElementById('gval' ).innerHTML = (c[1]/255*100).toFixed(0) + '%';
        document.getElementById('bval' ).innerHTML = (c[2]/255*100).toFixed(0) + '%';

        let rgbstr;
        _ll = document.getElementById('ll').checked;
        _rr = document.getElementById('rr').checked;
        if(_ll && _rr) { rgbstr = 'B'; }
        else if(!_ll && !_rr) { rgbstr = 'X'; }
        else{
                 if(_ll) { rgbstr = 'L'; }
            else if(_rr) { rgbstr = 'R'; }
        }
        connection.send('R'+ c[0] + rgbstr);
        connection.send('G'+ c[1] + rgbstr);
        connection.send('B'+ c[2] + rgbstr);
    }
    background(0);
    image(img, 0, 0);
    img.resize(width, 0);
    let t = 300;
    if(_rr) rightcolor = c;
    if(_ll) leftcolor = c;
    noFill(); strokeWeight(t);
    // stroke(c);
    // circle(width/2, height/2, width+t+20);
    stroke(rightcolor);
    arc(width/2, height/2, width+t+20, width+t+20, -PI/2, PI/2);
    stroke(leftcolor);
    arc(width/2, height/2, width+t+20, width+t+20, PI/2, -PI/2);
}

function setHue(hue) { // Set the RGB LED to a given hue (color) (0° = Red, 120° = Green, 240° = Blue)
    hue %= 360;                   // hue is an angle between 0 and 359°
    var radH = hue*3.142/180;   // Convert degrees to radians
    var rf, gf, bf;

    if(hue>=0 && hue<120){        // Convert from HSI color space to RGB              
        rf = cos(radH*3/4);
        gf = sin(radH*3/4);
        bf = 0;
    } else if(hue>=120 && hue<240){
        radH -= 2.09439;
        gf = cos(radH*3/4);
        bf = sin(radH*3/4);
        rf = 0;
    } else if(hue>=240 && hue<360){
        radH -= 4.188787;
        bf = cos(radH*3/4);
        rf = sin(radH*3/4);
        gf = 0;
    }
    rr = rf*rf*1023;
    gg = gf*gf*1023;
    bb = bf*bf*1023;
}

function leftRight(x){
    if(x==0){
        document.getElementById('ll').checked = true;
        document.getElementById('rr').checked = false;
    }
    else if(x==1){
        document.getElementById('ll').checked = false;
        document.getElementById('rr').checked = true;
    }
    if(x==2){
        document.getElementById('ll').checked = true;
        document.getElementById('rr').checked = true;
    }
}