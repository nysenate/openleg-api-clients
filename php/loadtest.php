<?php
/**
 * Testing script for loading various OpenLeg XML samples and parsing the results.
 * Usage: php loadtest.php [options]
 * 
 * Available options are:
 *   --class: the type of object to list (default bill)
 *   --values: a comma-separated list of values to return from the object
 *   --source: the server from which to retrieve results (default 'staging')
 *
 * Example:
 *   php loadtest.php --class=meeting --values=id --source=production
 */

include_once 'openleg.inc';
//class can be(B for Bills, M for meetings, C for calendars, T for transcripts, A for actions, V for votes)
$params = array('class' => 'bill', 'source' => 'staging', 'values' => array('id'));
if ($_GET) {
  if (isset($_GET['class'])){
    $params['class']=$_GET['class'];
  }
  if (isset($_GET['source'])){
    $params['source']=$_GET['source'];	
  }
  if (isset($_GET['values'])){
    $params['values']=$_GET['values'];	
  }
}
else
{
  $params = array_merge($params, parseArgs($argv));
}
$params['values'] = explode(',', str_replace(' ', '', $params['values']));

test($params);

function test($params)
{
  $source = ($params['source'] == 'production') ? OPENLEG_ROOT : OPENLEG_STAGING;
  $values = $params['values'];
  $class = $params['class'];
  if ($class == 'floorCalendar') {
    $path = getcwd() . "/xml_samples/calendar/floor-calendar" . ($source=='staging' ? '.staging' : '' ) . ".xml";
  }
  else if ($class == 'activeCalendar') {
    $path = getcwd() . "/xml_samples/calendar/active-calendar" . ($source=='staging' ? '.staging' : '' ) . ".xml";
  }
  else {
    $path = getcwd() . "/xml_samples/$class/$class" . ($source=='staging' ? '.staging' : '' ) . ".xml";
  }
  print "$path\n";
  if (($source = fopen($path, 'r')) == FALSE) {
    print "Could not open $path for reading.\n";
    return;
  }
  $xml = fread($source, filesize($path));
  fclose($source);
  switch($class) {
    case 'FC':
    case 'floorCalendar':
      $object = new OpenLegFloorCalendar(NULL, $source, $xml);
      break;
    case 'AC':
    case 'activeCalendar':
      $object = new OpenLegActiveCalendar(NULL, $source, $xml);
      break;
    case 'T':
    case 'transcript':
      $object = new OpenLegTranscript(NULL, $source, $xml);
      break;
    case 'A':
    case 'action':
      $object = new OpenLegAction(NULL, $source, $xml);
      break;
    case 'V':
    case 'vote':
      $object = new OpenLegVote(NULL, $source, $xml);
      break;
    case 'B':
    case 'bill':
      $object = new OpenLegBill(NULL, $source, $xml);
      break;
    case 'M':
    case 'meeting':
    default:
      $object = new OpenLegMeeting(NULL, $source, $xml);
      break;
  }
  foreach ($values as $value) {
    if ($value == '*') {
      $availableValues = $object->availableValues();
      foreach ($availableValues as $availableValue) {
        print "$availableValue: " . print_r($object->{$availableValue}(),TRUE) . "\n";
      }
    }
    else if ($value == 'availableValues') {
      $availableValues = $object->availableValues();
      print 'Available values: ' . implode(', ', $availableValues) . "\n";
    }
    else {
      print "$value: " . $object->{$value}() . "\n";
    }
  }
}

/**
 * parseArgs Command Line Interface (CLI) utility function.
 * @usage               $args = parseArgs($_SERVER['argv']);
 * @author              Patrick Fisher <patrick@pwfisher.com>
 * @source              https://github.com/pwfisher/CommandLine.php
 */
function parseArgs($argv)
{
    array_shift($argv); $o = array();
    foreach ($argv as $a){
        if (substr($a,0,2) == '--'){ $eq = strpos($a,'=');
            if ($eq !== false){ $o[substr($a,2,$eq-2)] = substr($a,$eq+1); }
            else { $k = substr($a,2); if (!isset($o[$k])){ $o[$k] = true; } } }
        else if (substr($a,0,1) == '-'){
            if (substr($a,2,1) == '='){ $o[substr($a,1,1)] = substr($a,3); }
            else { foreach (str_split(substr($a,1)) as $k){ if (!isset($o[$k])){ $o[$k] = true; } } } }
        else { $o[] = $a; } }
    return $o;
}
