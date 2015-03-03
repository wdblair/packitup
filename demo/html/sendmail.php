<?PHP
$email = file_get_contents("email/butoday.html");
$headers = "MIME-Version: 1.0" . "\r\n";
$headers .= "Content-type:text/html;charset=UTF-8" . "\r\n";
$headers .= "From: butoday@bu.edu" . "\r\n";
mail("ec700bravoa@yahoo.com", "testing", $email, $headers);
?>
