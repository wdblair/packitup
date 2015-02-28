<?PHP
$remote_ip = $_SERVER['REMOTE_ADDR'];
$user_agent = $_SERVER['HTTP_USER_AGENT'];
$fp = fopen('access.txt', 'a+');
fwrite($fp, $remote_ip.":".$user_agent."\n");
fclose($fp);
header("Content-type: image/jpeg");
$image=imagecreatefromgif('track.gif');
imagegif($image);
?>
