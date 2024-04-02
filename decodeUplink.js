function decodeUplink(input) {
  
  var str = ""
  var strBuf = ""
  
  function dec2bin(dec) {
    return (dec >>> 0).toString(2);
  }
  
  for (i=0; i < 4; i++) {
    strBuf=dec2bin(input.bytes[i], 2);
    
    if(strBuf.length < 8){
      for (j=0; j < 8 - strBuf.length; j++){
        strBuf = "0" + strBuf 
      }
    }
    
    str+=strBuf
  }
  
  num=parseInt(str, 2)/100;
  
  return {
    data: {
      Temperatura: num + "°C"
    },
    warnings: [],
    errors: []
  };
}