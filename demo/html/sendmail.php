<?PHP
$email1 = file_get_contents("email/alert.html");
$email2 = file_get_contents("email/butoday.html");
$headers = "MIME-Version: 1.0" . "\r\n";
$headers .= "Content-type:text/html;charset=UTF-8" . "\r\n";
$headers .= "From: butoday@bu.edu" . "\r\n";
if (count($argv) < 2) {
	die("Please specify either -fingerprint or -infect option.\n");
}
if ($argv[1] == "-fingerprint") {
	mail("ec700bravoa@yahoo.com", "BU Emergency Alert", $email1, $headers);
} else if ($argv[1] == "-infect") {
	mail("ec700bravoa@yahoo.com", "Giant Hamster Invades BU Beach", $email2, $headers);
}
?>
