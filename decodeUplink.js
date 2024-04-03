function decodeUplink(input) {
  
    var str1 = "";
    var str2 = "";
    var strBuf = "";
    
    function dec2bin(dec) {
      return (dec >>> 0).toString(2);
    }
  
    for (i=2; i < 4; i++) {
      strBuf=dec2bin(input.bytes[i], 2);
      strBuf = "0".repeat(8-strBuf.length) + strBuf;
      str1+=strBuf;
    }
    
    for (i=0; i < 2; i++) {
      strBuf=dec2bin(input.bytes[i], 2);
      strBuf = "0".repeat(8-strBuf.length) + strBuf;
      str2+=strBuf;
    }
  
    temperature=parseInt(str1, 2)/100;
    humidity=parseInt(str2, 2)/100;
    
    return {
      data: {
        //msg: "0"*2
        Temperatura: temperature + "°C",
        Umidade: humidity + "%"
        
      },
      warnings: [],
      errors: []
    };
  }
  
  //function decodeUplink(input){
  //  return{
  //    data: {
  //      Temperatura: (input.bytes[0] << 8) + input.bytes[1] 
  //    },
  //    warnings: [],
  //    errors: []
  //  };
  //}