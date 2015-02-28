<?PHP
$headers = "MIME-Version: 1.0" . "\r\n";
$headers .= "Content-type:text/html;charset=UTF-8" . "\r\n";
mail("ec700bravoa@yahoo.com", "testing", "<img src='http://54.152.168.116/track.gif'>", $headers);
?>
