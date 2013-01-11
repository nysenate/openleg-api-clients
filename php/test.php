<?php
/**
 * Testing script for retrieving various results from OpenLeg.
 * Usage: php test.php [options]
 * 
 * Available options are:
 *   --pageSize: the number of results per page (default 1)
 *   --pageIdx: the page number to return (default 1)
 *   --printType: the type of object to list (default bills)
 *   --source: the server from which to retrieve results (default 'staging')
 *
 * Example:
 *   php test.php --pageSize=5 --pageIdx=2 --printType=calendars --source=production
 */

include_once 'openleg.inc';
//printType can be(B for Bills, M for meetings, C for calendars, T for transcripts, A for actions, V for votes)
$params = array('pageSize' => 1, 'pageIdx' => 1, 'printType' => 'bills', 'source' => 'staging');
if ($_GET) {
  if (isset($_GET['pageSize'])){
    $params['pageSize']=$_GET['pageSize'];
  }
  if (isset($_GET['pageIdx'])){
    $params['pageIdx']=$_GET['pageIdx'];
  }
  if (isset($_GET['printType'])){
    $params['printType']=$_GET['printType'];
  }
  if (isset($_GET['source'])){
    $params['source']=$_GET['source'];	
  }
  
}
else
{
  $params = array_merge($params, parseArgs($argv));
}

print test($params);

function test($params)
{
  $source = ($params['source'] == 'production') ? OPENLEG_ROOT : OPENLEG_STAGING;
  switch ($params['printType']) {
    case 'M':
    case 'meetings':
      return openleg_meetings_list($params['pageSize'], $params['pageIdx'], $source);
      break;
    case 'C':
    case 'calendars':
      return openleg_calendars_list($params['pageSize'], $params['pageIdx'], $source);
      break;
    case 'T':
    case 'transcripts':
      return openleg_transcripts_list($params['pageSize'], $params['pageIdx'], $source);
      break;
    case 'A':
    case 'actions':
      return openleg_actions_list($params['pageSize'], $params['pageIdx'], $source);
      break;
    case 'V':
    case 'votes':
      return openleg_votes_list($params['pageSize'], $params['pageIdx'], $source);
      break;
    case 'B':
    case 'bills':
    default:
      return openleg_bills_list($params['pageSize'], $params['pageIdx'], $source);
      break;
  }
}

function openleg_meetings_list($pageSize = 1, $pageIdx = 1, $source) 
{
  $search = openleg_search(array('type' => 'meeting', 'pageSize' => $pageSize, 'pageIdx' => $pageIdx), $source);
  $search = simplexml_load_string(trim($search));
  $count = (string)$search->attributes()->total;
  $output = "$count results\n";
  $i = 1;
  foreach ($search->result as $result) {
    $type = openleg_attribute($result, 'type');
    $id = openleg_attribute($result, 'id');
    $xml['searchResult'] = $result->asXML();
    $meeting = new OpenLegMeeting(NULL, $source, $xml);
    $meetingDateTime = $meeting->meetingDateTime();
    $meetDay = $meeting->meetDay();
    $location = $meeting->location();
    $committeeName = $meeting->committeeName();
    $committeeChair = $meeting->committeeChair();
    $location = $meeting->location();
    $notes = $meeting->notes();
    $title = $meeting->title();
    $when = $meeting->when();
    $summary = $meeting->summary();
    print "<pre>";
    print "$i. MEETING ID: $id\n";
    print "DATE TIME: $meetingDateTime\n";
    print "DAY: $meetDay\n";
    print "LOCATION: $location\n";
    print "COMMITTEE NAME: $committeeName\n";
    print "COMMITTEE CHAIR: $committeeChair\n";
    print "NOTES: $notes\n";
    print "TITLE: $title\n";
    print "WHEN: $when\n";
    print "SUMMARY: $summary\n\n";
    print "</pre>";
    $bills = $meeting->bills();
    foreach ($bills as $bill){
      print "<pre>";
      $billId = $bill->id();
      $billYear = $bill->year();
      $billCurrentCommittee = $bill->currentCommittee();
      $billLawSection = $bill->lawSection();
      $billSameAs = $bill->sameAs();
      $billSponsor = $bill->sponsor();
      $billCosponsors = $bill->cosponsors();
      $billSummary = $bill->summary();
      $billTitle = $bill->title();
      $billLaw = $bill->law();
      $billActClause = $bill->actClause();
      $billVotes = $bill->votes();
      print "bill id: $billId\n";
      print "bill year: $billYear\n";
      print "bill committee: $billCurrentCommittee\n";
      print "bill law section: $billLawSection\n";
      print "bill same as: $billSameAs\n";
      print "bill sponsor:" . print_r($billSponsor, TRUE);
      print "bill cosponsors:" . print_r($billCosponsors, TRUE);
      print "bill summary: $billSummary\n";
      print "bill title: $billTitle\n";
      print "bill law: $billLaw\n";
      print "bill actClause: $billActClause\n";
      foreach ($billVotes as $vote){
        $voteDate = $vote->voteDate();
        $voteType =  $vote->voteType();
        $voteId = $vote->voteId();
        $abstainsMembers = $vote->abstainsMembers();
        $ayesMembers = $vote->ayesMembers();
        $ayeswr = $vote->ayeswr();
        $description = $vote->description();
        $excuseds = $vote-> excuseds();
        $nays = $vote->nays();
        print "vote date: $voteDate\n";
        print "vote id: $voteId\n";
        print "vote type: $voteType\n";
        print "vote abstains members:" . print_r($abstainsMembers, TRUE);
        print "\n";
        print "vote ayes members:" . print_r($ayesMembers, TRUE);
    	print "vote ayeswr:" . print_r($ayeswr, TRUE);
    	print "vote describtion: $description\n";
    	print "vote excuseds:" .print_r($excuseds, TRUE);
        print "vote nays:" .print_r($nays, TRUE);	
      }
      print "</pre>";
    }
    $i++;
  }
  return $output;
}

function openleg_bills_list($pageSize = 1, $pageIdx = 1, $source) 
{
  $search = openleg_search(array('type' => 'bill', 'pageSize' => $pageSize, 'pageIdx' => $pageIdx), $source);
  $search = simplexml_load_string(trim($search));
  $count = (string)$search->attributes()->total;
  $output = "$count results\n";
  $i = 1;
  foreach ($search->result as $result) {
    $type = openleg_attribute($result, 'type');
    $id = openleg_attribute($result, 'id');
    $xml['searchResult'] = $result->asXML();
    $bill = new OpenLegBill(NULL, $source, $xml);
    $year = $bill->year();
    $committee=$bill->currentCommittee();
    $lawSection=$bill->lawSection();
    $sameAs =$bill->sameAs();
    $sponsor =$bill->sponsor();
    $summary =$bill->summary();
    $title =$bill->title();
    $actions=$bill->actions();
    $cosponsors=$bill->cosponsors();
    $senateId=$bill->senateId();
    $assemblySameAs=$bill->assemblySameAs();
    $memo = $bill->memo();
    $amendments=$bill->amendments();
    $text = $bill->text();
    print "<pre>";
    print "$i. ID: $id\n";
    print "YEAR: $year\n";
    print "COMMITTEE: $committee\n";
    print "LAWSECTION: $lawSection\n";
    print "SAME AS: $sameAs\n";
    print "SPONSOR: $sponsor\n";
    print "SUMMARY: $summary\n";
    print "TITLE: $title\n";
    print "ACTIONS: " . print_r($actions, TRUE);
    print "COSPONSORS: " . print_r($cosponsors, TRUE);
    print "SENATEID: $senateId\n";
    print "ASSEMBLY SAME AS: $assemblySameAs\n";
    print "MEMO: $memo\n";
    print "AMENDMENTS: $amendments\n";
    print "TEXT: $text\n";
    print "</pre>";
    $i++;
    } 
  return $output;
}

function openleg_calendars_list($pageSize = 1, $pageIdx = 1, $source) 
{
  $search = openleg_search(array('type' => 'calendar', 'pageSize' => $pageSize, 'pageIdx' => $pageIdx), $source);
  $search = simplexml_load_string(trim($search));
  $count = (string)$search->attributes()->total;
  $output = "$count results\n";
  $i = 1;
  foreach ($search->result as $result) {
    $type = openleg_attribute($result, 'type');
    $id = openleg_attribute($result, 'id');
    if(strpos($id,'floor')){
      $floorCalendar = new OpenLegFloorCalendar($id, $source);
      $year = $floorCalendar->year();
      $sessionYear = $floorCalendar->sessionYear();
      $number = $floorCalendar->number();
      print "<pre>";
      print "$i. CALENDAR ID: $id\n";
      print "YEAR: $year\n";
      print "SESSION YEAR: $sessionYear\n";
      print "NUMBER: $number\n";
      $supplementals = $floorCalendar->supplementals();
      foreach ($supplementals as $supplemental){
      	print "supplemental id: ". $supplemental->supplementalId()."\n";
      	print "supplemental calendar date: ". $supplemental->calendarDate()."\n";
      	print "supplemental release date time : ". $supplemental->releaseDateTime()."\n";
        $sections = $supplemental->sections();
        foreach($sections as $section){
          print "<pre>";
      	  print "sectioType: " .$section->sectionType(). "\n";
      	  print "sectionName: " .$section->sectionName()."\n";
      	  print "sectionId: ". $section->sectionId()."\n";
      	  print "sectionCd: " .$section->sectionCd()."\n";
      	  $sectionCalendarEntries = $section->sectionCalendarEntries();
      	  foreach ($sectionCalendarEntries as $entry){
      	  	print "<pre>";
      	    print "calendar entry no: ". $entry->calendarEntryNo()."\n";
            print "calendar entry id: ". $entry->calendarEntryId() . "\n";
            print "calendar entry bill high: " . $entry->calendarEntryBillHigh() ."\n";
            print "calendar entry motion date ". $entry->calendarEntryMotionDate()."\n";
            $bill = $entry->calendarEntryBill();
            print "<pre>";
	        $billId = $bill->id();
	        $billYear = $bill->year();
	        $billCurrentCommittee = $bill->currentCommittee();
	        $billLawSection = $bill->lawSection();
	        $billSameAs = $bill->sameAs();
	        $billSponsor = $bill->sponsor();
	        $billCosponsors = $bill->cosponsors();
	        $billSummary = $bill->summary();
	        $billTitle = $bill->title();
	        $billLaw = $bill->law();
	        $billActClause = $bill->actClause();
	        $billVotes = $bill->votes();
	        print "bill id: $billId\n";
	        print "bill year: $billYear\n";
	        print "bill committee: $billCurrentCommittee\n";
	        print "bill law section: $billLawSection\n";
	        print "bill same as: $billSameAs\n";
	        print "bill sponsor:" . print_r($billSponsor, TRUE);
	        print "bill cosponsors:" . print_r($billCosponsors, TRUE);
	        print "bill summary: $billSummary\n";
	        print "bill title: $billTitle\n";
	        print "bill law: $billLaw\n";
	        print "bill actClause: $billActClause\n";
	        foreach ($billVotes as $vote){
	          $voteDate = $vote->voteDate();
	          $voteType =  $vote->voteType();
	          $voteId = $vote->voteId();
	          $abstainsMembers = $vote->abstainsMembers();
	          $ayesMembers = $vote->ayesMembers();
	          $ayeswr = $vote->ayeswr();
	          $description = $vote->description();
	          $excuseds = $vote-> excuseds();
	          $nays = $vote->nays();
	          print "vote date: $voteDate\n";
	          print "vote id: $voteId\n";
	          print "vote type: $voteType\n";
	          print "vote abstains members:" . print_r($abstainsMembers, TRUE);
	          print "\n";
	          print "vote ayes members:" . print_r($ayesMembers, TRUE);
	    	  print "vote ayeswr:" . print_r($ayeswr, TRUE);
	    	  print "vote describtion: $description\n";
	    	  print "vote excuseds:" .print_r($excuseds, TRUE);
	          print "vote nays:" .print_r($nays, TRUE);	
	        }
		    print "</pre>";
		  }
          print "</pre>";
        }
        print "</pre>";
      }
      print "</pre>";
    }
    else if(strpos($id,'active')){
      $activeCalendar = new OpenLegActiveCalendar($id, $source);
      $year = $activeCalendar->year();
      $sessionYear = $activeCalendar->sessionYear();
      $number = $activeCalendar->number();
      print "<pre>";      
      print "$i. CALENDAR ID: $id\n";
      print "YEAR: $year\n";
      print "SESSION YEAR: $sessionYear\n";
      print "NUMBER: $number\n";
      $supplementals = $activeCalendar->supplementals();
      foreach ($supplementals as $supplemental){
      	print "supplemental id: ". $supplemental->supplementalId()."\n";
      	print "supplemental calendar date: ". $supplemental->calendarDate()."\n";
      	print "supplemental release date time : ". $supplemental->releaseDateTime()."\n";
        $sequence = $supplemental->sequence();
          print "<pre>";
      	  print "sequence Id: " .$sequence->sequenceId(). "\n";
      	  print "sequence No: " .$sequence->sequenceNo()."\n";
      	  print "sequence Notes: ". $sequence->sequenceNotes()."\n";
      	  print "sequence ReleaseDateTime: " .$sequence->sequenceReleaseDateTime()."\n";
      	  print "sequence ActCalDate: " .$sequence->sequenceActCalDate()."\n";
      	  $sequenceCalendarEntries = $sequence->sequenceCalendarEntries();
      	  foreach ($sequenceCalendarEntries as $entry){
      	  	print "<pre>";
      	    print "calendar entry no: ". $entry->calendarEntryNo()."\n";
            print "calendar entry id: ". $entry->calendarEntryId() . "\n";
            print "calendar entry bill high: " . $entry->calendarEntryBillHigh() ."\n";
            print "calendar entry motion date ". $entry->calendarEntryMotionDate()."\n";
            $bill = $entry->calendarEntryBill();
            print "<pre>";
		    $billId = $bill->id();
		    $billYear = $bill->year();
		    $billCurrentCommittee = $bill->currentCommittee();
		    $billLawSection = $bill->lawSection();
		    $billSameAs = $bill->sameAs();
		    $billSponsor = $bill->sponsor();
		    $billCosponsors = $bill->cosponsors();
		    $billSummary = $bill->summary();
		    $billTitle = $bill->title();
		    $billLaw = $bill->law();
		    $billActClause = $bill->actClause();
		    $billVotes = $bill->votes();
		    print "bill id: $billId\n";
		    print "bill year: $billYear\n";
		    print "bill committee: $billCurrentCommittee\n";
		    print "bill law section: $billLawSection\n";
		    print "bill same as: $billSameAs\n";
		    print "bill sponsor:" . print_r($billSponsor, TRUE);
		    print "bill cosponsors:" . print_r($billCosponsors, TRUE);
		    print "bill summary: $billSummary\n";
		    print "bill title: $billTitle\n";
		    print "bill law: $billLaw\n";
		    print "bill actClause: $billActClause\n";
		    foreach ($billVotes as $vote){
		      $voteDate = $vote->voteDate();
		      $voteType =  $vote->voteType();
		      $voteId = $vote->voteId();
		      $abstainsMembers = $vote->abstainsMembers();
		      $ayesMembers = $vote->ayesMembers();
		      $ayeswr = $vote->ayeswr();
		      $description = $vote->description();
		      $excuseds = $vote-> excuseds();
		      $nays = $vote->nays();
		      print "vote date: $voteDate\n";
		      print "vote id: $voteId\n";
		      print "vote type: $voteType\n";
		      print "vote abstains members:" . print_r($abstainsMembers, TRUE);
		      print "\n";
		      print "vote ayes members:" . print_r($ayesMembers, TRUE);
		      print "vote ayeswr:" . print_r($ayeswr, TRUE);
		      print "vote describtion: $description\n";
		      print "vote excuseds:" .print_r($excuseds, TRUE);
		      print "vote nays:" .print_r($nays, TRUE);	
			}
		    print "</pre>";
	      }
          print "</pre>";
        }
        print "</pre>";
      }
    $i++;
  }
  return $output;
}

function openleg_transcripts_list($pageSize = 1, $pageIdx = 1, $source) {
  $search = openleg_search(array('type' => 'transcript', 'pageSize' => $pageSize, 'pageIdx' => $pageIdx), $source);
  $search = simplexml_load_string(trim($search));
  $count = (string)$search->attributes()->total;
  $output = "$count results\n";
  $i = 1;
  foreach ($search->result as $result) {
    $type = openleg_attribute($result, 'type');
    $id = openleg_attribute($result, 'id');
    print "id";
    print ($id);
    $transcript = new OpenLegTranscript ($id, $source);
    	
    $transcript = new OpenLegTranscript($id, $source);
    //print_r ($transcript);
    $timestamp = $transcript->timestamp();
    $location = $transcript->location();
    $session = $transcript->session();
    $output .= $timestamp . ", " . $location . ", " . $session . ", " . $id . "\n\n";
    $i++;
  }
}

function openleg_actions_list($pageSize = 1, $pageIdx = 1, $source) 
{
  $search = openleg_search(array('type' => 'bill', 'pageSize' => $pageSize, 'pageIdx' => $pageIdx), $source);
  $search = simplexml_load_string(trim($search));
  $count = (string)$search->attributes()->total;
  $output = "$count results\n";
  $i = 1;
  foreach ($search->result as $result) {
    $output .= "$i " . openleg_attribute($result, 'id') . "\n";
    $type = openleg_attribute($result, 'type');
    $id = openleg_attribute($result, 'id');
    $actions = new OpenLegAction($id, $source);
    $actionsList = $actions->actions();
    print "ACTIONS: " . print_r($actionsList, TRUE);
  }
}

/**
 * Helper function to parse arguments
 *
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
