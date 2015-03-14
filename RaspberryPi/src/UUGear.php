<?php

define('DATA_TYPE_VOID',	0x01);
define('DATA_TYPE_STRING',	0x02);
define('DATA_TYPE_INTEGER',	0x03);
define('DATA_TYPE_FLOAT',	0x04);

$functionMap = [

'attachUUGearDevice'	=> [ 'request' => 1,	'return' => DATA_TYPE_INTEGER,	'paramCount' => 1, 'paramType1' => DATA_TYPE_STRING ],
'detachUUGearDevice'	=> [ 'request' => 2,	'return' => DATA_TYPE_VOID,		'paramCount' => 1, 'paramType1' => DATA_TYPE_STRING ],
'resetUUGearDevice'		=> [ 'request' => 217,	'return' => DATA_TYPE_VOID,		'paramCount' => 1, 'paramType1' => DATA_TYPE_STRING ],

'setPinModeAsInput'		=> [ 'request' => 10,	'return' => DATA_TYPE_VOID,		'paramCount' => 2, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER ],
'setPinModeAsOutput'	=> [ 'request' => 11,	'return' => DATA_TYPE_VOID,		'paramCount' => 2, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER ],
'setPinLow'				=> [ 'request' => 12,	'return' => DATA_TYPE_VOID,		'paramCount' => 2, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER ],	
'setPinHigh'			=> [ 'request' => 13,	'return' => DATA_TYPE_VOID,		'paramCount' => 2, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER ],
'getPinStatus'			=> [ 'request' => 14,	'return' => DATA_TYPE_INTEGER,	'paramCount' => 2, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER ],
'analogWrite'			=> [ 'request' => 15,	'return' => DATA_TYPE_VOID,		'paramCount' => 3, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER, 'paramType3' => DATA_TYPE_INTEGER ],
'analogRead'			=> [ 'request' => 16,	'return' => DATA_TYPE_INTEGER,	'paramCount' => 2, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER ],
'analogReference'		=> [ 'request' => 17,	'return' => DATA_TYPE_VOID,		'paramCount' => 2, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER ],	

'attachServo'			=> [ 'request' => 25,	'return' => DATA_TYPE_VOID,		'paramCount' => 2, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER ],
'writeServo'			=> [ 'request' => 26,	'return' => DATA_TYPE_VOID,		'paramCount' => 3, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER, 'paramType3' => DATA_TYPE_INTEGER ],
'readServo'				=> [ 'request' => 27,	'return' => DATA_TYPE_INTEGER,	'paramCount' => 2, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER ],
'detachServo'			=> [ 'request' => 28,	'return' => DATA_TYPE_VOID,		'paramCount' => 2, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER ],

'readDHT'				=> [ 'request' => 41,	'return' => DATA_TYPE_INTEGER,	'paramCount' => 2, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER ],
'readSR04'				=> [ 'request' => 42,	'return' => DATA_TYPE_FLOAT,	'paramCount' => 3, 'paramType1' => DATA_TYPE_STRING, 'paramType2' => DATA_TYPE_INTEGER, 'paramType3' => DATA_TYPE_INTEGER ],

];

function sendRequest($req, $respNeeded) {
    $resp = '';
    $timeout = 10;
    $socket = stream_socket_client('unix:///tmp/uugear_socket_broker.sock', $errorno, $errorstr, $timeout);
    stream_set_timeout($socket, $timeout);

    if(!fwrite($socket, $req)) {
		echo('Socket write error!');
	}
	if ($respNeeded) {
	    if (!($resp = fread($socket, 1024))) {
			echo('Socket read error!');
	    } else {
	    	echo($_REQUEST['callback'].'('.$resp.')');
	    }
	}
}

if (isset($_REQUEST['function'])) {
	
	$function = $functionMap[$_REQUEST['function']];
	
	if ($function) {
		$request = pack('CC', $function['request'], $function['return']);
		
		$missParam = false;
		
		for ($i = 1; $i <= $function['paramCount']; $i ++) {
			if (!isset($_REQUEST['paramValue'.$i])) {
				echo 'Parameter #'.$i.' is missing!';
				$missParam = true;
				break;
			}
			switch ($function['paramType'.$i]) {
				case DATA_TYPE_STRING:
					$request .= pack('CC', DATA_TYPE_STRING, strlen($_REQUEST['paramValue'.$i])).$_REQUEST['paramValue'.$i];
					break;
				case DATA_TYPE_INTEGER:
					$integerData = $_REQUEST['paramValue'.$i];
					$request .= pack('CC', DATA_TYPE_INTEGER, strlen($integerData)).$integerData;
					break;
				case DATA_TYPE_FLOAT:
					$floatData = $_REQUEST['paramValue'.$i];
					$request .= pack('CC', DATA_TYPE_FLOAT, strlen($floatData)).$floatData;
					break;
			}
		}
		
		if (!$missParam) {
			sendRequest($request, $function['return'] != DATA_TYPE_VOID);
		}
	}
}

?>