<?php
if (isset($_GET['serve']) && $_GET['serve'] === 'code'){
	header ("Content-Type: text/plain");
	readfile(__FILE__);
	exit();
}
?>
<HTML>
<HEAD>
<style type="text/css">
body {background-color: #eee} 
a.showcode {color: #787;position: fixed; font-weight: bold; }
h3 {text-align: center;display: block;border: 1px solid; margin: 2px 0; background-color: #a88 }
div {margin: 0 10%}
div.alt { margin:0 10px;display: none;position: absolute;background: white; border: 1px solid}
a.showalt:hover div.alt { display: block}
a.showalt {text-decoration: none; color: black}
table {border-collapse:collapse; margin: 2px}
table thead {margin: 2px 0; background-color: #a88}
td {border:1px solid}
td.subdata { border:none; }
td.subdata table { margin-left: 10px }
td.directive { background-color: #88a }
td.args { background-color: #8a8 }
</style>
</HEAD>
<BODY>
<a class="showcode" href="<?=$_SERVER['PHP_SELF'] ?>?serve=code">Click here to see the PHP code</a><br/>
<div>
<H3>Apache vhosts page</H3>
<?php
function print_subtree($arr)
{
	foreach ($arr as $name => $args_array) {
		foreach ($args_array as $data) {
			echo "<tr><td class=\"directive\">$name</td><td class=\"args\"><a class=\"showalt\" href=\"\">$data[args]";
			echo "<div class=\"alt\">File: $data[filename]<br/>Line: $data[line_num]</div>" ;
			echo "</a></td></tr>" ;
			if (isset($data["subtree"])) {
				echo '<tr><td class="subdata" colspan="2"><table>';
				print_subtree($data["subtree"]);
				echo '</table></td></tr>' ;
			}		
		}
	}
}

function print_vhost_config ($arr) {
	foreach ($arr as $data) {
		echo <<<EOF
<table><thead>
<tr><td>Name:</td><td>
<a href="" class="showalt">$data[args]
<div class="alt">
File: $data[filename]<br/>
Line: $data[line_num]
</div></a>
</td></tr>
</thead><tbody>
EOF
;
		print_subtree($data["subtree"]);
		echo "</tbody></table>\n";
	}		
}
$conf = apacheaccessor_get_conf();
print_vhost_config ($conf["VirtualHost"]);
?>
</div>
</BODY></HTML>
