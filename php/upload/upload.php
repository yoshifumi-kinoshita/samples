<?php

if($_FILES){

	var_dump( $_FILES );
	$uploaddir = '/var/www/html/uploads/';
	$uploadfile = $uploaddir . basename($_FILES['userfile']['name']);
	if (move_uploaded_file($_FILES['userfile']['tmp_name'], $uploadfile)) {
	    echo "succeed";
	} else {
	    echo "fail";
	}

}


?>


<form enctype="multipart/form-data" method="POST">
    <input type="hidden" name="MAX_FILE_SIZE" value="30000" />
    <input name="userfile" type="file" />
    <input type="submit" value="upload" />
</form>
