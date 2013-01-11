<?php
// $Id$

/**
 * @file
 * Common functionality for querying the OpenLeg API.
 *
 * Documentation of the OpenLeg API is available at:
 * http://senatedev.senate.state.ny.us/wiki/legxml/Notes_on_OpenLeg_APIPHP_library
 */

define('OPENLEG_ROOT', 'http://open.nysenate.gov/legislation/');
define('OPENLEG_STAGING', 'http://open-staging.nysenate.gov/legislation/');

/**
 * Superclass for all OpenLeg classes.
 */
abstract class OpenLegObject
{
  // Property declarations
  protected $source = OPENLEG_ROOT;
  protected $xml = NULL; // the XML of a full item
  protected $sxe = NULL; // the full object parsed into a SimpleXMLElement (Simple XML Element abbreviates as sxe)
  protected $xmlSearchResult = NULL;  // the XML of an item retrieved via a searchResult search
  protected $sxeSearchResult = NULL;  // a searchResult search item parsed into a SimpleXML element

  // Method declarations

  public function get() 
  {
    $this->sxe = NULL;
  }
  // Return an HTML version of the object
  public function html() 
  {
    return '';
  }
  // Return an XML version of the object
  public function xml()
  {
    return '';
  }
  public function getData()
  {
    if ($this->xml) {
      return array(
        'full' => $this->xml,
        'source' => $this->source,
      );
    }
    else if ($this->sxe) {
      return array(
        'full' => $this->sxe->asXml(),
        'source' => $this->source,
      );
    }
    else if ($this->searchResult) {
      return array(
        'searchResult' => $this->sxeSearchResult,
        'source' => $this->source,
      );
    }
    else if ($this->sxeSearchResult) {
      return array(
        'full' => $this->sxeSearchResult->asXml(),
        'source' => $this->source,
      );
    }
  }
  public function loadData($data)
  {
    if (isset($data['full'])) {
      $this->xml = $data['full'];
      $this->sxe = simplexml_load_string(trim($this->xml));
    }
    if (isset($data['searchResult'])) {
      $this->xmlsearchResult = $data['searchResult'];
      $this->sxeSearchResult = simplexml_load_string(trim($this->xmlsearchResult));
    }
    if (isset($data['source'])) {
      $this->source = $data['source'];
    }
  }

  // return an array listing the names of available values from this class
  public function availableValues() {
    return array();
  }
  // Return a CSV version of the object
  public function csv() 
  {
    return '';
  }
  // Return a JSON version of the object
  public function json() 
  {
    return '';
  }
  // Return an RSS version of the object
  public function rss() 
  {
    return '';
  }
}

/**
 * For handling lists of OpenLeg objects.
 */
class OpenLegItemList extends OpenLegObject
{
  // property declarations
  protected $count; // the total number of items returned by the search

  // method declarations/overrides
  public function get() 
  {
    $this->sxe = NULL;
  }
  public function item($n) 
  {
    // return the $nth item in the list
  }
}

/**
 * For handling searchResult searches retrieved via the Search API.
 */
class OpenLegSearch extends OpenLegItemList
{
  // property declarations
  public $count; // the total number of items returned by the search

  // method declarations/overrides
  public function get() 
  {
    $this->sxe = NULL;
  }
}

/**
 * For handling listings retrieved via the Collections API.
 * DEPRECATED?
 */
class OpenLegCollection extends OpenLegItemList
{
  // property declarations

  // method declarations/overrides
  public function get() 
  {
    $this->sxe = NULL;
  }
}

/**
 * For handling OpenLeg Items.
 *
 */
class OpenLegItem extends OpenLegObject
{
  // property declarations
  protected $type = NULL;
  protected $id = NULL;
  protected $xmlSubItem=NULL;
  protected $sxeSubItem=NULL;

  // method declarations/overrides
  public function availableValues() {
    return array('id', 'type');
  }

  public function __construct($type, $id=NULL, $source=OPENLEG_ROOT, $xml=NULL)
  {
    $this->type = $type;
    $this->id = $id;
    $this->source = $source;
    $this->xml = $xml;

    if ($xml) {
      if (is_array($xml)) {
        $this->loadData($xml);
      }
      else {
        $this->loadData(array('full' => $xml));
      }
    }
    else if ($this->id) {
      $this->get();
    }
  }

  public function getData()
  {
    $data = parent::getData();
    if (!is_null($this->type)) {
      $data['type'] = $this->type;
    }
    if (!is_null($this->id)) {
      $data['id'] = $this->id;
    }
    if (!is_null($this->xmlSubItem)) {
      $data['subitem'] = $this->xmlSubItem;
    }
    else if (!is_null($this->sxeSubItem)) {
      $data['subitem'] = $this->sxeSubItem->asXml();
    }
    return $data;
  }
  public function loadData($data)
  {
    parent::loadData($data);
    if (isset($data['type'])) {
      $this->type = $data['type'];
    }
    if (isset($data['id'])) {
      $this->id = $data['id'];
    }
    if (isset($data['subitem'])) {
      $this->xmlSubItem = $data['subitem'];
      $this->sxeSubItem = simplexml_load_string(trim($this->xmlSubItem));
    }
    if (is_null($this->id)) {
      if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
        $this->id = $this->sxe->attributes()->id;
      }
      else if ($this->sxeSearchResult && get_class($this->sxe) == 'SimpleXMLElement') {
        $this->id = $this->sxeSearchResult->attributes()->id;
      }
    }
  }
  
  public function get() 
  {  
    $this->xml = openleg_curl_request($this->source . 'api/1.0/xml/' . $this->type . '/' . urlencode($this->id));
    $this->xml = str_replace("\0","",$this->xml); // hacky fix for XML bug
    $this->sxe = simplexml_load_string(trim($this->xml));
  }
}

/**
 * A single meeting.
 */
class OpenLegMeeting extends OpenLegItem
{
  // property declarations
  private $attrs;

  // These properties are for caching available object values
  private $meetingDateTime;
  private $meetDay;
  private $location;
  private $committeeName;
  private $committeeChair;
  private $bills;
  private $notes;
  private $votes;
  private $title; //INCLUDED ONLY IN searchResult SEARCH
  private $when; //INCLUDED ONLY IN searchResult SEARCH
  private $summary; //INCLUDED ONLY IN searchResult SEARCH

  // method declarations/overrides
  public function availableValues() {
    return array_merge(
      parent::availableValues(), 
      array('meetingDateTime', 'meetDay', 'location', 'committeeName', 'committeeChair', 'bills', 'notes', 'title', 'when', 'summary')
    );
  }

  public function __construct($id=NULL, $source=OPENLEG_ROOT, $xml=NULL)
  {
    parent::__construct('meeting', $id, $source, $xml);
    if (isset($this->sxeSearchResult)){
      $this->id = (string)$this->sxeSearchResult->attributes()->id;
      $this->attrs= $this->sxeSearchResult->attributes();
  	}
  }
  
  public function type() 
  {
    return $this->type;
  }
  
  public function id() 
  {
    return $this->id;
  }
  
  public function source() 
  {
    return $this->source;
  }
  
  /* This function is used to retrieve the meeting's date and time for each meeting. 
   * This function takes no parameter and returns meetingDateTime as a string. For example: "2010-02-24T11:00:00-05:00".
   */  
  public function meetingDateTime()
  {
  	// If meetingDateTime property is already set, then get its saved value.
    if (!is_null($this->meetingDateTime)) {
      return $this->meetingDateTime;
    }
   // DATA IS NOT AVAILABLE FROM searchResult YET.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['meetingDateTime'])){
        return $this->meetingDateTime =(string)$this->sxeSearchResult->attributes()->meetingDateTime;
      }
      else {
        if (!isset($this->sxe)){
          $this->get();
        }
        return $this->meetingDateTime =(string)$this->sxe->attributes()->meetingDateTime;
      }
    }   
    // To get meetingDateTime from the full object.
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->meetingDateTime =(string)$this->sxe->attributes()->meetingDateTime;
    }
  }
  
  /* This function is used to retrieve the meeting's day for each meeting. 
   * This function takes no parameter and returns meet Day as a string. For example: "Wednesday".
   */
  public function meetDay() 
  {
  	// If meetDay property is already set, then get its saved value.
    if (!is_null($this->meetDay)) {
      return $this->meetDay;
    }
   // DATA IS NOT AVAILABLE FROM searchResult YET.
     else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
     	if (isset($this->attrs['meetDay'])){
          return $this->meetDay =(string)$this->sxeSearchResult->attributes()->meetday;
        }
        else {
          if (!isset($this->sxe)){
            $this->get();
          }
          return $this->meetDay = (string)$this->sxe->attributes()->meetday;	
        }
     }
    // To get meetDay from the full object.
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->meetDay = (string)$this->sxe->attributes()->meetday;
    }
  }

  /* Retrieves the location for each meeting. 
   * It takes no parameter and returns location as a string. For example: "Hearing Room A".
   */
  public function location() 
  {
  	// If location property is already set, then get its saved value.
    if (!is_null($this->location)) {
      return $this->location;
    }
    // To get location from the searchResult object.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['location'])){
        return $this->location = (string)$this->sxeSearchResult->attributes()->location;
      }
      else {
        if (!isset($this->sxe)){
          $this->get();
        }
        return (string)$this->sxe->attributes()->location;
      }
    }
    // To get location from the full object.
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return (string)$this->sxe->attributes()->location;
    }
  }
  
  /* Retrieves the commitee Name of each meeting. 
   * It takes no parameter and returns committee name as a string. For example: "Cultural Affairs, Tourism, Parks and Recreation".
   */  
  public function committeeName()
  {
  	// If committee property is already set, then get its saved value.
    if (!is_null($this->committeeName)) {
      return $this->committeeName;
    }
    // To get committe from the searchResult object.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['committee'])){	
        return $this->committeeName = (string)$this->sxeSearchResult->attributes()->committee;
      }
      else {
        if (!isset($this->sxe)){
          $this->get();
        }
        return $this->committeeName = (string)$this->sxe->attributes()->committeeName;      	
      }
    }
    // To get committe from the full object.
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->committeeName = (string)$this->sxe->attributes()->committeeName;
    }
  }
  
  /* Retrieves the commitee chair name for each meeting. 
   * It takes no parameter and returns committee cahir name as a string. For example: "Jose M. Serrano".
   */
  public function committeeChair() 
  {
  	// If chair property is already set, then get its saved value.
    if (!is_null($this->committeeChair)) {
      return $this->committeeChair;
    }
    // To get chair from the searchResult object.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['chair'])){
        return $this->committeeChair = (string)$this->sxeSearchResult->attributes()->chair;
      }
      else {
        if (!isset($this->sxe)){
          $this->get();
        }
        return $this->committeeChair = (string)$this->sxe->attributes()->committeeChair;
      }
    }
    // To get chair from the full object.
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->committeeChair = (string)$this->sxe->attributes()->committeeChair;
    }
  }
  
  /* Retrieves the bills discussed in each meeting. 
   * It takes no parameter and returns bills as an array.
   */ 
  public function bills() 
  {
  	// If bills property is already set, then get its saved value.
    if (!is_null($this->bills)) {
      return $this->bills;
    }
    // To get bills from the full object.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['bills'])){
        return $this->bills = (string)$this->sxeSearchResult->attributes()->bills;
      }
      else {
        if (!isset($this->sxe)){
          $this->get();
        }
        $bills = array();
        foreach($this->sxe->bills->bill as $bill){
          $billXml = $bill->asXML();
          $bills[]  = new OpenLegVotedBill($bill->attributes()->id,$this->source, $billXml);
    	}
        return $this->bills = $bills;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
    	$bills = array();
    	foreach($this->sxe->bills->bill as $bill){
          $billXml = $bill->asXML();
          $bills[]  = new OpenLegVotedBill($bill->attributes()->id,$this->source, $billXml);
    	}
        return $this->bills = $bills;
    }
  }
  
  /* Retrieves notes for each meeting. 
   * It takes no parameter and returns notes as a string.
   */ 
  public function notes() 
  {
  	// If notes property is already set, then get its saved value.
    if (!is_null($this->notes)) {
      return $this->notes;
    }
    // DATA IS NOT AVAILABLE FROM searchResult YET.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['notes'])){
        return $this->notes = (string)$this->sxeSearchResult->attributes()->notes;
      }
      else {
        if (!isset($this->sxe)){
          $this->get();
        }
        return $this->notes = (string)$this->sxe->notes;
      }
    }
    // To get notes from the full object.  
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->notes = (string)$this->sxe->notes;
    }
  }
  
  public function votes()
  {
    $votes = array();
    foreach ($this->sxe->votes as $entry){
      $voteXml = $entry->asXML();
      $votes[] = new OpenLegVote($voteXml);
    }
    return $votes;	
  }

  /* Retrieves the title of each meeting. 
   * It takes no parameter and returns title as a string.
   */ 
  public function title()
  {
  	// If title property is already set, then get its saved value.
    if (!is_null($this->title)) {
      return $this->title;
    }
    // To get title from the searchResult object.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['title'])){
        return $this->title = (string)$this->sxeSearchResult->attributes()->title;
      }
    }
  }
  
  public function when()
  {
  	// If when property is already set, then get its saved value.
    if (!is_null($this->when)) {
      return $this->when;
    }
    // To get when from the searchResult object.
    if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['when'])){
        return $this->when = (string)$this->sxeSearchResult->attributes()->when;
      }
    }
  }
  
  /* Retrieves the summary of each meeting. 
   * It takes no parameter and returns summary as a string.
   */
  public function summary()
  {
  	// If summary property is already set, then get its saved value.
    if (!is_null($this->summary)) {
      return $this->summary;
    }
    // To get summary from the searchResult object.
    if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['summary'])){
        return $this->summary = (string)$this->sxeSearchResult->attributes()->summary;
      }
    }
  }
}

/**
 * A single voted bill.
 */
class OpenLegBill extends OpenLegItem
{
  // property declarations
  private $attrs;

  // These properties are for caching bill values.
  private $year;
  private $currentCommittee;
  private $lawSection;
  private $sameAs;
  private $sponsor;
  private $summary;
  private $title;
  private $actions;
  private $cosponsors;
  private $senateId;
  private $assemblySameAs;
  private $amendments;
  private $memo;
  private $text;

  public function availableValues()
  {
    return array_merge(
      parent::availableValues(), 
      array(
        'id',
        'year',
        'currentCommittee',
        'lawSection',
        'sameAs',
        'sponsor',
        'summary',
        'title',
        'actions',
        'cosponsors',
        'senateId',
        'assemblySameAs',
        'amendments',
        'memo',
        'text',
      )
    );
  }

  public function __construct($id=NULL, $source=OPENLEG_ROOT, $xml=NULL)
  {
    parent::__construct('bill', $id, $source, $xml);
    if (isset($this->sxeSearchResult)) {
      $this->id = (string)$this->sxeSearchResult->attributes()->id;
      $this->attrs= $this->sxeSearchResult->attributes();
    }
  }
  
  public function type() 
  {
    return $this->type;
  }
  public function id() 
  {
    return $this->id;
  }
  public function source() 
  {
    return $this->source;
  }
  public function year() 
  {
    if (!is_null($this->year)) {
      return $this->year;
    }
    // The year is currently available in searchResult search on the live site but not yet on staging.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['year'])){
        return $this->year = (string)$this->sxeSearchResult->attributes()->year;
      }
      else {
        if (!isset($this->sxe)){
          $this->get();
        }
        return $this->year = (string)$this->sxe->bill->attributes()->year;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->year = (string)$this->sxe->bill->attributes()->year;
    }
  }
  
  public function currentCommittee() 
  {
    if (!is_null($this->currentCommittee)) {
      return $this->currentCommittee;
    }
    if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['committee'])){
        return $this->currentCommittee = (string)$this->sxeSearchResult->attributes()->committee;
      }
      else {
        if (!isset($this->sxe)){
          $this->get();
        }
          return $this->currentCommittee = (string)$this->sxe->bill->committee;
      } 
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->currentCommittee = (string)$this->sxe->bill->committee;
    }
  }
  
  public function lawSection() 
  {
    if (!is_null($this->lawSection)) {
      return $this->lawSection;
    }
    // The lawSection is currently only available in search results from staging.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['lawSection'])){
        return $this->lawSection= (string)$this->sxeSearchResult->attributes()->lawSection;
      }
      else {
        if (!isset($this->sxe)){
          $this->get();
        }
        return $this->lawSection = (string)$this->sxe->bill->attributes()->lawSection;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->lawSection = (string)$this->sxe->bill->attributes()->lawSection;
    }
  }
  
  public function sameAs() 
  {
    if (!is_null($this->sameAs)) {
      return $this->sameAs;
    }
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['sameAs'])){
        return $this->sameAs = (string)$this->sxeSearchResult->attributes()->sameas;
      }
      else {
      	if (!isset($this->sxe)){
          $this->get();
      	}
        return $this->sameAs = (string)$this->sxe->bill->attributes()->sameAs;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->sameAs = (string)$this->sxe->bill->attributes()->sameAs;
    }
  }
  
  public function sponsor() 
  {
    if (!is_null($this->sponsor)) {
      return $this->sponsor;
    }
  else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
    	if (isset($attrs['sponsor'])){
    		return $this->sponsor= (string)$this->sxeSearchResult->attributes()->sponsor;
    	}
    	else {
    	  if (!isset($this->sxe)){
    	    $this->get();
    	  }
          return $this->sponsor = (string)$this->sxe->bill->attributes()->sponsor;
    	}
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->sponsor = (string)$this->sxe->bill->attributes()->sponsor;
    }
  }
  
  public function summary() 
  {
    if (!is_null($this->summary)) {
      return $this->summary;
    }
    if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['summary'])){
        return $this->summary =(string)$this->sxeSearchResult->attributes()->summary;
      }
      else {
        if (!isset($this->sxe)){
    	  $this->get();
    	}
    	return $this->summary = (string)$this->sxe->summary;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->summary = (string)$this->sxe->summary;
    }
  }
  
  public function title() 
  {
    if (!is_null($this->title)) {
      return $this->title;
    }
    if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['title'])){
        return $this->title = (string)$this->sxeSearchResult->attributes()->title;
      }
      else {
        if (!isset($this->sxe)){
    	  $this->get();
    	}
        return $this->title = (string)$this->sxe->attributes()->title;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->title = (string)$this->sxe->attributes()->title;
    }
  }
  
  public function actions() 
  {
    if (!is_null($this->actions)) {
      return $this->actions;
    }
    // The actions aren't currently available in searchResult search. THE CODE NEEDS RECHECKING AFTER IT'S ADDED.
    else if ($this->sxeSearchResult && get_class($this->sxe) == 'SimpleXMLElement') {
      if (isset($this->attrs['actions'])){
      	return $this->actions = (string)$this->sxeSearchResult->attributes()->actions;
      }
      else {
        if (!isset($this->sxe)){
    	  $this->get();
    	}
        $output=array();
        foreach ($this->sxe->bill->actions as $actions) {
          $internalOutput=array();
          foreach ($actions->action as $action){
            $internalOutput['timestamp']= (string)$action->attributes()->timestamp;
            $internalOutput['action']= (string) $action;
            $output[]=$internalOutput;
          }
        }
        return $this->actions = $output;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      $output=array();
      foreach ($this->sxe->bill->actions as $actions) {
        $internalOutput=array();
        foreach ($actions->action as $action){
          $internalOutput['timestamp']= (string)$action->attributes()->timestamp;
          $internalOutput['action']= (string) $action;
          $output[]=$internalOutput;
        }
      }
      return $this->actions = $output;
    }
  }
  
  public function cosponsors()
  {
    if (!is_null($this->cosponsors)) {
      return $this->cosponsors;
    }
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['cosponsors'])){
        return $this->cosponsors =(string)$this->sxeSearchResult->attributes()->cosponsors;
      }
      else {
        if (!isset($this->sxe)){
    	  $this->get();
    	}
    	$output=array();
        foreach ($this->sxe->bill->cosponsors->cosponsor as $cosponsor){
          $output[]=(string) $cosponsor;
        }
        return $this->cosponsors =$output;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      $output=array();
      foreach ($this->sxe->bill->cosponsors->cosponsor as $cosponsor){
        $output[]=(string) $cosponsor;
      }
      return $this->cosponsors =$output;
    }
  }
  
  public function senateId()
  {
    if (!is_null($this->senateId)) {
      return $this->senateId;
    }
    // The senateId isn't currently available in searchResult search.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['senateId'])){
        return $this->senateId =(string)$this->sxeSearchResult->attributes()->senateId;
      }
      else {
        if (!isset($this->sxe)){
    	  $this->get();
    	}
    	return $this->senateId = (string)$this->sxe->bill->attributes()->senateId;
      }
    }
    	
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->senateId = (string)$this->sxe->bill->attributes()->senateId;
    }
  }
  
  public function assemblySameAs()
  {
    if (!is_null($this->assemblySameAs)) {
      return $this->assemplySameAs;
    }
    // The assemplySameAs isn't currently available in searchResult search.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['assemblySameAs'])){
        return $this->assemblySameAs =(string)$this->sxeSearchResult->attributes()->assemblySameAs;
      }
      else {
        if (!isset($this->sxe)){
    	  $this->get();
    	}
        return $this->assemblySameAs = (string)$this->sxe->bill->attributes()->assemblySameAs;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->assemblySameAs = (string)$this->sxe->bill->attributes()->assemblySameAs;
    }
  }
  
  public function memo()
  {
    if (!is_null($this->memo)) {
      return $this->memo;
    }
    // The memo isn't currently available in searchResult search.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['memo'])) {
        return $this->memo =(string)$this->sxeSearchResult->attributes()->memo;
      }
      else {
        if (!isset($this->sxe)) {
          $this->get();
        }
        // the memo doesn't appear as an element in the current bill example XML
        return $this->memo = (string)$this->sxe->bill->memo;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->memo = (string)$this->sxe->bill->memo;
    }
  }
  
// I found only one amendments in the bill that has data. It has only id. When executing it gave the error Node no longer exists 
  public function amendments(){
    if (!is_null($this->amendments)) {
      return $this->amendments;
    }
    // The amendments aren't currently available in searchResult search, so we have to get the full item.
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['amendments'])){
        return $this->amendments =(string)$this->sxeSearchResult->attributes()->amendments;
      }
      else {
        if (!isset($this->sxe)){
    	  $this->get();
    	}
    	return $this->amendments = (string) $this->sxe->bill->amendments;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->amendments = (string) $this->sxe->bill->amendments;
    }
  }
  
  public function text(){
    if (!is_null($this->text)) {
      return $this->text;
    }
    // The amendments aren't currently available in searchResult search, so we have to get the full item.
    else if (!isset($this->sxe)){
      $this->get();
      return $this->text = (string) $this->sxe->bill->text;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->text = (string) $this->sxe->bill->text;
    }
  }
}

class OpenLegVotedBill extends OpenLegBill
{
  private $year;
  private $currentCommittee;
  private $lawSection;
  private $sameAs;
  private $sponsor;
  private $cosponsors;
  private $summary;
  private $title;
  private $law;
  private $actClause;
  private $votes;
  

  public function __construct($id, $source, $sourceBill)
  {
    if (isset($sourceBill)) {
      $this->xml = $sourceBill;
      $this->sxe = simplexml_load_string(trim($this->xml));
      $this->id = (string)$this->sxe->attributes()->id;
    }
  }
  
  public function availableValues()
  {
    return array_merge(
      parent::availableValues(), 
      array(
        'law',
        'actClause',
        'votes',
      )
    );
  }

  public function year() 
  {
    if (!is_null($this->year)) {
      return $this->year;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement'){
      return $this->year = (string)$this->sxe->attributes()->year;
    }
  }
  
  public function currentCommittee() 
  {
    if (!is_null($this->currentCommittee)) {
      return $this->currentCommittee;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement'){
      return $this->currentCommittee = (string)$this->sxe->currentCommittee;
    }  
  }
  
  public function lawSection() 
  {
    if (!is_null($this->lawSection)) {
      return $this->lawSection;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement'){
      return $this->lawSection = (string)$this->sxe->lawSection;
    }
  }
  
  public function law() 
  {
    if (!is_null($this->law)) {
      return $this->law;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement'){
      return $this->law = (string)$this->sxe->law;
    }
  }
  
  public function sameAs() 
  {
    if (!is_null($this->sameAs)) {
      return $this->sameAs;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement'){
      return $this->sameAs = (string)$this->sxe->sameAs;
    }
  }
  
  public function sponsor() 
  {
    if (!is_null($this->sponsor)) {
      return $this->sponsor;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement'){
      $output=array();
      $output['position']= (string)$this->sxe->sponsor->attributes()->position;
      $output['fullname']= (string) $this->sxe->sponsor->attributes()->fullname;
      $output['id']= (string) $this->sxe->sponsor->id;
      $output['guid']= (string) $this->sxe->sponsor->guid;
      $output['branch'] = (string)$this->sxe->sponsor->attributes()->branch;
      $output['contactInfo'] = (string)$this->sxe->sponsor->attributes()->contactInfo;
      return $this->sponsor = $output;
    }
  }
  
  public function summary() 
  {
    if (!is_null($this->summary)) {
      return $this->summary;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement'){
      return $this->summary = (string)$this->sxe->summary;
    }
  }
  
  public function title() 
  {
    if (!is_null($this->title)) {
      return $this->title;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement'){
      return $this->title = (string)$this->sxe->title;
    }
  }
  
  public function cosponsors()
  {
    if (!is_null($this->cosponsors)) {
      return $this->cosponsors;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      $cosponsors = array();
      $output=array();
        foreach ($this->sxe->coSponsors as $entry){
          $cosponsor['position']= (string)$entry->attributes()->position;
          $cosponsor['fullname']= (string) $entry->attributes()->fullname;
          $cosponsor['id']= (string) $entry->id;
          $cosponsor['guid']= (string) $entry->guid;
          $cosponsor['branch'] = (string) $entry->attributes()->branch;
          $cosponsor['contactInfo'] = (string) $entry->attributes()->contactInfo;
          $cosponsors[]=$cosponsor;
        }
      }
      return $this->cosponsors = $cosponsors;
   }
   
  public function actClause()
  {
    if (!is_null($this->actClause)) {
      return $this->actClause;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->actClause = (string)$this->sxe->actClause;
    }
  } 
  
  public function votes()
  {
    $votes = array();
    foreach ($this->sxe->votes as $entry){
      $billXml = $entry->asXML();
      $votes[] = new OpenLegVote($billXml);
    }
    return $votes;	
  }
}

/**
 * A single calendar.
 */
class OpenLegCalendar extends OpenLegItem
{
  // property declarations
  private $year;
  private $sessionYear;
  private $number;
  private $supplementals;

  public function availableValues()
  {
    return array_merge(
      parent::availableValues(), 
      array(
        'year',
        'sessionYear',
        'number',
        'supplementals',
      )
    );
  }

  public function __construct($id=NULL, $source=OPENLEG_ROOT, $xml=NULL)
  {
    parent::__construct('calendar', $id, $source, $xml);
  }
  
  public function type() 
  {
    return $this->type;
  }
  
  public function id() 
  {
    return $this->id;
  }
  
  public function source() 
  {
    return $this->source;
  }

  public function year()
  {
    if (!is_null($this->year)) {
      return $this->year;
    }
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['year'])){
        return $this->year =(string)$this->sxeSearchResult->attributes()->year;
      }
      else {
        if (!isset($this->sxe)){
    	  $this->get();
    	}
    	return $this->year = (string)$this->sxe->attributes()->year;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->year = (string)$this->sxe->attributes()->year;
    }
  }
  
  public function sessionYear()
  {
    if (!is_null($this->sessionYear)) {
      return $this->sessionYear;
    }
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['sessionYear'])){
        return $this->sessionYear =(string)$this->sxeSearchResult->attributes()->sessionYear;
      }
      else {
        if (!isset($this->sxe)){
    	  $this->get();
    	}
    	return $this->sessionYear = (string)$this->sxe->attributes()->sessionYear;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->sessionYear = (string)$this->sxe->attributes()->sessionYear;
    }
  }
  
  public function number()
  {
  if (!is_null($this->number)) {
      return $this->number;
    }
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      if (isset($this->attrs['no'])){
        return $this->number =(string)$this->sxeSearchResult->attributes()->no;
      }
      else {
        if (!isset($this->sxe)){
    	  $this->get();
    	}
    	return $this->number = (string)$this->sxe->attributes()->no;
      }
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->number = (string)$this->sxe->attributes()->no;
    }
  }
  
}

class OpenLegActiveCalendar extends OpenLegCalendar
{
  // property declarations
  private $supplementals;
  
  public function availableValues()
  {
    return array_merge(
      parent::availableValues(), 
      array(
        'supplementals',
      )
    );
  }

  /*public function xml($xml=NULL) 
  {
    // If XML is provided, load it and construct the object's data as a SimpleXMLElement
    if ($xml) {
      if (!is_array($xml)) {
        $this->xml = $xml;
      }
      else {
        if (isset($xml['full'])){
        	$this->xml = $xml['full'];
        }
        if (isset($xml['searchResult'])){
        	$this->xmlSearchResult = $xml['searchResult'];
      	}
      }
      // NEED RECHECKING I FIXED THE ERROR BY CHECKING IF $THIS->XML IS NOT ARRAY!
      if (!is_array($this->xml)) {
        $this->sxe = simplexml_load_string(trim($this->xml));
        $this->id = 'cal-active-' . sprintf('%05s', $this->sxe->attributes()->no) . '-' . $this->sxe->attributes()->sessionYear;
      }
      else if ($this->xmlSearchResult) {
        $this->sxeSearchResult = simplexml_load_string($this->xmlSearchResult);
        $this->id = (string)$this->sxeSearchResult->attributes()->id;
      }
      /*else if ($this->xmlSubItem) {
        $this->sxeSubItem = simplexml_load_string($this->xmlSubItem);
        $this->id = (string)$this->sxeSubItem->attributes()->id;
      }*/
    //}
    // If no XML is provided, RETURN the XML that was used to construct the object.
   /* else if ($this->xml) {
      return $this->xml;
    }
    // This probably isn't needed, but maybe there will be situations where the XML doesn't exist but it can be
    // derived from the corresponding SimpleXML element.
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->sxe->asXml();
    }
    // If the object's full XML doesn't exist, pass back an associative array for just the searchResult search xml.
    else if ($this->xmlSearchResult) {
      return array('searchResult' => $this->xmlSearchResult);
    }
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      return array('searchResult' => $this->sxeSearchResult->asXml());
    }
    else if ($this->sxeSubItem && get_class($this->sxeSubItem) == 'SimpleXMLElement') {
      return array('subitem' => $this->sxeSubItem->asXml());
    }
  }*/

  public function supplementals()
  {
  if (!is_null($this->supplementals)) {
      return $this->supplementals;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      $supplementals = array();
      foreach ($this->sxe->supplementals->supplemental as $supplemental){
        $xmlEntry = $supplemental->asXML();
        $supplementals[] = new OpenLegSupplementalSequence($xmlEntry);	
      }
      return $this->supplementals = $supplementals;
    } 	
  }
  
}

class OpenLegFloorCalendar extends OpenLegCalendar
{
  //protected $sxe;
  // property declarations
  private $supplementals;
  
  public function availableValues()
  {
    return array_merge(
      parent::availableValues(), 
      array(
        'supplementals',
      )
    );
  }
  
/* 
  public function xml($xml=NULL) 
  {
    // If XML is provided, load it and construct the object's data as a SimpleXMLElement
    if ($xml) {
      if (!is_array($xml)) {
        $this->xml = $xml;
      }
      else {
        if (isset($xml['full'])){
        	$this->xml = $xml['full'];
        }
        if (isset($xml['searchResult'])){
        	$this->xmlSearchResult = $xml['searchResult'];
      	}
      }
      if (!is_array($this->xml)) {
        $this->sxe = simplexml_load_string(trim($this->xml));
        $this->id = 'cal-floor-' . sprintf('%05s', $this->sxe->attributes()->no) . '-' . $this->sxe->attributes()->sessionYear;
      }
      else if ($this->xmlSearchResult) {
        $this->sxeSearchResult = simplexml_load_string($this->xmlSearchResult);
        $this->id = (string)$this->sxeSearchResult->attributes()->id;
      }
    }*/
    
    // If no XML is provided, RETURN the XML that was used to construct the object.
/*    else if ($this->xml) {
      return $this->xml;
    }
    // This probably isn't needed, but maybe there will be situations where the XML doesn't exist but it can be
    // derived from the corresponding SimpleXML element.
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->sxe->asXml();
    }
    // If the object's full XML doesn't exist, pass back an associative array for just the searchResult search xml.
    else if ($this->xmlSearchResult) {
      return array('searchResult' => $this->xmlSearchResult);
    }
    else if ($this->sxeSearchResult && get_class($this->sxeSearchResult) == 'SimpleXMLElement') {
      return array('searchResult' => $this->sxeSearchResult->asXml());
    }
    else if ($this->sxeSubItem && get_class($this->sxeSubItem) == 'SimpleXMLElement') {
      return array('subitem' => $this->sxeSubItem->asXml());
    }
  }*/

  public function supplementals()
  {
  if (!is_null($this->supplementals)) {
      return $this->supplementals;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      $supplementals = array();
      foreach ($this->sxe->supplementals->supplemental as $supplemental){
        $xmlEntry = $supplemental->asXML();
        $supplementals[] = new OpenLegSupplementalSection($xmlEntry);
      }
      return $this->supplementals = $supplementals;
    } 	
  }
}

class OpenLegCalendarSupplemental extends OpenLegItem {

  private $supplementalId;
  private $calendarDate;
  private $releaseDateTime;
  
  public function __construct($sourceSupplemental)
  {
    if (isset($sourceSupplemental)) {
      $this->xml = $sourceSupplemental;
      $this->sxe = simplexml_load_string(trim($this->xml));
      $this->id = (string)$this->sxe->attributes()->supplementalId;
    }
  }

  public function availableValues()
  {
    return array_merge(
      parent::availableValues(), 
      array(
        'supplementalId',
        'calendarDate',
        'releaseDateTime',
      )
    );
  }

  public function supplementalId()
  {
    if (!is_null($this->supplementalId)) {
      return $this->supplementalId;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return  $this->supplementalId = (string) $this->sxe->attributes()->supplementalId;
      }
  }
  
  public function calendarDate()
  {
    if (!is_null($this->calendarDate)) {
      return $this->calendarDate;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->calendarDate = (string)$this->sxe->calendarDate;
    }
  }
  
  public function releaseDateTime()
  {
    if (!is_null($this->releaseDateTime)) {
      return $this->releaseDateTime;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return  $this->releaseDateTime = (string)$this->sxe->releaseDateTime;
    }
  }
}

class OpenLegSupplementalSection extends OpenLegCalendarSupplemental {
  private $sections;

  public function availableValues()
  {
    return array_merge(
      parent::availableValues(), 
      array(
        'sections',
      )
    );
  }
  
  public function sections()
  {
    if (!is_null($this->sections)) {
      return $this->sections;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      $sections=array();
      foreach($this->sxe->sections->section as $section){
        $xmlEntry = $section->asXML(); 
        $sections[] = new OpenLegCalendarSection($xmlEntry);
      }
      return $this->sections = $sections;
    }
  }	
}

class OpenLegSupplementalSequence extends OpenLegCalendarSupplemental {
  private $sequence;


  public function availableValues()
  {
    return array_merge(
      parent::availableValues(), 
      array(
        'squence',
      )
    );
  }
  
  public function sequence()
  {
    if (!is_null($this->sequence)) {
      return $this->sequence;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      $xmlEntry = $this->sxe->sequence->asXML(); 
      return $this->sequence = new OpenLegCalendarSequence($xmlEntry);
    }
  }
}

class OpenLegCalendarSection extends OpenLegItem {
  
  private $sectionType;
  private $sectionName;
  private $sectionId;
  private $sectionCd;
  private $sectionCalendarEntries;
  
  public function __construct($sourceSection)
  {
    if (isset($sourceSection)) {
      $this->xml = $sourceSection;
      $this->sxe = simplexml_load_string(trim($this->xml));
      $this->id = (string)$this->sxe->attributes()->id;
    }
  }
  
  public function availableValues()
  {
    return array_merge(
      parent::availableValues(), 
      array(
        'sectionType',
        'sectionName',
        'sectionId',
        'sectionCd',
        'sectionCalendarEntries',
      )
    );
  }
   
  public function sectionType()
  {
    if (!is_null($this->sectionType)) {
      return $this->sectionType;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->sectionType = (string) $this->sxe->attributes()->type;
    }
  }
  
  public function sectionName()
  {
    if (!is_null($this->sectionName)) {
      return $this->sectionName;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return  $this->sectionName = (string) $this->sxe->attributes()->name;
    }
  }
  
  public function sectionId()
  {
    if (!is_null($this->sectionId)) {
      return $this->sectionId;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return  $this->sectionId = (string) $this->sxe->attributes()->id;
    }
  }
  
  public function sectionCd()
  {
    if (!is_null($this->sectionCd)) {
      return $this->sectionCd;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return  $this->sectionCd = (string) $this->sxe->attributes()->cd;
    }
  }
  
  public function sectionCalendarEntries()
  {
  	if (!is_null($this->sectionCalendarEntries)) {
      return $this->sectionCalendarEntries;
  	}
  	else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      $output = array();	
      foreach ($this->sxe->calNos->calNo as $calNo){
        $xmlEntry = $calNo->asXML(); 
      	$output[] = new OpenLegCalendarEntry($xmlEntry); 
      }
      return $this->sectionCalendarEntries = $output;
  	}
  }
}

class OpenLegCalendarSequence extends OpenLegItem {
	
  private $sequenceId;
  private $sequenceNo;
  private $sequenceNotes;
  private $sequenceReleaseDateTime;
  private $sequenceActCalDate;
  private $sequenceCalendarEntries;

  public function __construct($sourceSequence)
  {
    if (isset($sourceSequence)) {
      $this->xml = $sourceSequence;
      $this->sxe = simplexml_load_string(trim($this->xml));
      $this->id = (string)$this->sxe->attributes()->id;
    }
  }
  
  public function availableValues()
  {
    return array_merge(
      parent::availableValues(), 
      array(
        'sequenceId',
        'sequenceNo',
        'sequenceNotes',
        'sequenceReleaseDateTime',
        'sequenceActCalDate',
        'sequenceCalendarEntries',
      )
    );
  }
  
 public function sequenceId()
  {
  	if (!is_null($this->sequenceId)) {
      return $this->sequenceId;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->sequenceId = (string)$this->sxe->attributes()->id;
    }
  }
  
  public function sequenceNo()
  {
  	if (!is_null($this->sequenceNo)) {
      return $this->sequenceNo;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->sequenceNo = (string)$this->sxe->no;
    }
  }
  
  public function sequenceNotes()
  {
  	if (!is_null($this->sequenceNotes)) {
      return $this->sequenceNotes;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->sequenceNotes = (string)$this->sxe->notes;
    }
  }
  
  public function sequenceReleaseDateTime()
  {
  	if (!is_null($this->sequenceReleaseDateTime)) {
      return $this->sequenceReleaseDateTime;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
        return $this->sequenceReleaseDateTime = (string)$this->sxe->releaseDateTime;
      }
  }
    
  public function sequenceActCalDate()
  {
  	if (!is_null($this->sequenceActCalDate)) {
      return $this->sequenceActCalDate;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->sequenceActCalDate = (string)$this->sxe->actCalDate;
    }
  }
  
  public function sequenceCalendarEntries()
  {
  	if (!is_null($this->sequenceCalendarEntries)) {
      return $this->sequenceCalendarEntries;
  	}
  	else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      $output = array();	
      foreach($this->sxe->calendarEntries as $calendarEntries){
        $xmlEntry = $calendarEntries->asXML(); 
      	$output[] = new OpenLegCalendarEntry($xmlEntry); 
      }
      return $this->sequenceCalendarEntries = $output;
    }
  }

}

class OpenLegCalendarEntry extends OpenLegItem
{
  private $calendarEntryNo;
  private $calendarEntryId;
  private $calendarEntryBillHigh;
  private $calendarEntryMotionDate;
  private $calendarEntryBill;
  
  public function __construct($sourceEntry)
  {
    if (isset($sourceEntry)) {
      $this->xml = $sourceEntry;
      $this->sxe = simplexml_load_string(trim($this->xml));
      $this->id = (string)$this->sxe->attributes()->id;
    }
  }
  
  public function calendarEntryNo()
  {
  	if (!is_null($this->calendarEntryNo)) {
      return $this->calendarEntryNo;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->calendarEntryNo = (string) $this->sxe->attributes()->no;
    }
  }
  
  public function calendarEntryId()
  {
  	if (!is_null($this->calendarEntryId)) {
      return $this->calendarEntryId;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->calendarEntryId = (string) $this->sxe->attributes()->id;
    }
  }
  
  public function calendarEntryBillHigh()
  {
  	if (!is_null($this->calendarEntryBillHigh)) {
      return $this->calendarEntryBillHigh;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->calendarEntryBillHigh = (string) $this->sxe->attributes()->billHigh;
    }
  }
  
  public function calendarEntryMotionDate()
  {
  	if (!is_null($this->calendarEntryMotionDate)) {
      return $this->calendarEntryMotionDate;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return $this->calendarEntryMotionDate = (string) $this->sxe->attributes()->motionDate;
    }
  }
   
  public function calendarEntryBill()
  {
    if (!is_null($this->calendarEntryBill)) {
      return $this->calendarEntryBill;
    }
    else if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      $billXml = $this->sxe->bill->asXML();
      $bill  = new OpenLegVotedBill($this->sxe->bill->attributes()->id,$this->source, $billXml);
    }
    return $this->calendarEntryBill = $bill;
  }
}

/**
 * A single transcript.
 */
class OpenLegTranscript extends OpenLegItem
{
  // property declarations

  public function __construct($id=NULL, $source=OPENLEG_ROOT, $xml=NULL)
  {
    parent::__construct('transcript', $id, $source, $xml);
  }
  
  public function type() 
  {
    return $this->type;
  }
  
  public function id() 
  {
    return $this->id;
  }
  
  public function source() 
  {
    return $this->source;
  }
  
  public function timestamp()
  {
    if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return (string)$this->sxe->timestamp;
    }
  }
  
  public function location()
  {
    if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return (string)$this->sxe->location;
    }
  }
  
  public function session()
  {
    if ($this->sxe && get_class($this->sxe) == 'SimpleXMLElement') {
      return (string)$this->sxe->session;
    }
  }
  // method declarations/overrides
}

/**
 * A single vote.
 */
class OpenLegVote extends OpenLegItem
{
  protected $xmlVote;
  protected $sxeVote;
  
  private $voteDate;
  private $voteType;
  private $voteId;
  private $abstainsMembers;
  private $ayesMembers;
  private $ayeswr;
  private $description;
  private $excuseds;
  private $nays;
  
  public function getData()
  {
    $data = parent::getData();
    if (!is_null($this->xmlVote)) {
      $data['vote'] = $this->xmlVote;
    }
    else if (!is_null($this->sxeBill)) {
      $data['vote'] = $this->sxeVote->asXml();
    }
    return $data;
  }
  public function loadData($data)
  {
    parent::loadData($data);
    if (isset($data['vote'])) {
      $this->xmlVote = $data['vote'];
      $this->sxeVote = simplexml_load_string(trim($this->xmlVote));
    }
  }

  public function availableValues()
  {
    return array_merge(
      parent::availableValues(), 
      array(
        'voteDate',
        'voteType',
        'voteId',
        'abstainsMembers',
        'ayesMembers',
        'ayeswr',
        'description',
        'excuseds',
        'nays',
      )
    );
  }

  public function __construct($sourceVote)
  {
    if (isset($sourceVote)) {
      $this->xmlVote = $sourceVote;
      $this->sxeVote = simplexml_load_string(trim($this->xmlVote));
      $this->id = (string)$this->sxeVote->attributes()->id;
    }
  }
  
  public function type() 
  {
    return $this->sxeVote->voteType;
  }
  
  public function id() 
  {
    return $this->sxeVote->id;
  }
  
/*  public function source() 
  {
    return $this->sxeVote->source;
  }*/
  
  public function voteDate()
  {
    if (!is_null($this->voteDate)) {
      return $this->voteDate;
    }
    else if ($this->sxeVote && get_class($this->sxeVote) == 'SimpleXMLElement') {
      return  $this->voteDate= (string) $this->sxeVote->attributes()->voteDate;
    }
  }
  
  public function voteType()
  {
    if (!is_null($this->voteType)) {
      return $this->voteType;
    }
    else if ($this->sxeVote && get_class($this->sxeVote) == 'SimpleXMLElement') {
        return $this->voteType = (string) $this->sxeVote->attributes()->voteType;
    }
  }
  
  public function voteId()
  {
    if (!is_null($this->voteId)) {
      return $this->voteId;
    }
    else if ($this->sxeVote && get_class($this->sxeVote) == 'SimpleXMLElement') {
      return $this->voteId = (string) $this->sxeVote->attributes()->id;
    }
  }
  
  public function abstainsMembers()
  {
    if (!is_null($this->abstainsMembers)) {
      return $this->abstainsMembers;
    }
    else if ($this->sxeVote && get_class($this->sxeVote) == 'SimpleXMLElement') {
      $output=array();
      if ($this->sxeVote->abstains->member){
        foreach($this->sxeVote->abstains->member as $member){
          $output[]= (string) $member;
        }
        return $this->abstainsMembers = $output;
      }
    }
  }
  
  public function ayesMembers()
  {
    if (!is_null($this->ayesMembers)) {
      return $this->ayesMembers;
    }
    else if ($this->sxeVote && get_class($this->sxeVote) == 'SimpleXMLElement') {
      $output=array();
      if($this->sxeVote->ayes->member){	
        foreach($this->sxeVote->ayes->member as $member){
          $output[]= (string) $member;
        }
        return $this->ayesMembers = $output;
      }
    }
  }
  
  public function ayeswr()
  {
    if (!is_null($this->ayeswr)) {
      return $this->ayeswr;
    }    
    else if ($this->sxeVote && get_class($this->sxeVote) == 'SimpleXMLElement') {
      $output = array();
      if ($this->sxeVote->ayeswr){
        foreach ($this->sxeVote->ayeswr as $ayeswr){
          $output[]= (string) $ayeswr;
        }
        return $this->ayeswr = $output;
      }
    }
  }
  
  public function description()
  {
    if (!is_null($this->description)) {
      return $this->description;
    }
    else if ($this->sxeVote && get_class($this->sxeVote) == 'SimpleXMLElement') {
      return $this->description = (string) $this->sxeVote->description;
    }
  }
   
  public function excuseds()
  {
    if (!is_null($this->excuseds)) {
      return $this->excuseds;
    }
    else if ($this->sxeVote && get_class($this->sxeVote) == 'SimpleXMLElement') {
      $output=array();
      if ($this->sxeVote->excuseds->member) {
        foreach ($this->sxeVote->excuseds->member as $member){
          $output[]= (string) $member;
        }
      }
      return $this->excuseds = $output;
    }
  }
  
  public function nays()
  {
    if (!is_null($this->nays)) {
      return $this->nays;
    }
    else if ($this->sxeVote && get_class($this->sxeVote) == 'SimpleXMLElement') {
      $output=array();
      if ($this->sxeVote->nays->member){
        foreach($this->sxeVote->nays->member as $member){
          $output[]= (string) $member;
        }
        return $this->nays = $output;
      }
    }
  }
}

/**
 * A single action.
 */
class OpenLegAction extends OpenLegItem
{
  // property declarations
  //public $type = 'action';
  private $bill;
  private $actions;
  
  public function __construct($id=NULL, $source=OPENLEG_ROOT, $xml=NULL)
  {
    $this->bill = new openLegBill ($id, $source);
  }
  
  public function type() 
  {
    return $this->bill->type;
  }
  
  public function id() 
  {
    return $this->bill->id;
  }
  
  public function source() 
  {
    return $this->bill->source;
  }
  
  public function actions() 
  {
    if (!is_null($this->actions)) {
      return $this->actions;
    }
    else if ($this->bill->sxe && get_class($this->bill->sxe) == 'SimpleXMLElement') {
      return $this->actions = $this->bill->actions();
    }
  }
   // method declarations/overrides
}

function openleg_curl_request($url) 
{
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 15);
  curl_setopt($ch, CURLOPT_TIMEOUT, 15);
  $data = curl_exec($ch);
  curl_close($ch);
  return $data;
}

function openleg_retrieve($path, $source=OPENLEG_ROOT) 
{
  $xml = openleg_curl_request($source . 'api/1.0/xml/' . str_replace("%2F", "/", urlencode($path)));
  $xml = str_replace("\0","",$xml); // hacky fix for bad XML bug
  return simplexml_load_string(trim($xml));
}

function openleg_retrieve_xml($path, $source=OPENLEG_ROOT) 
{
  $xml = openleg_curl_request($source . 'api/1.0/xml/' . str_replace("%2F", "/", urlencode($path)));
  $xml = str_replace("\0","",$xml); // hacky fix for bad XML bug
  return $xml;
}

/**
 * Query OpenLeg via searchResult search.
 * 
 * @param $params
 *   An associative array of parameters. The search needs at least one parameter for either 'type' or 'term'. All
 *   other parameters are optional. Each value of The full list of parameters is:
 *     type => meeting, bill
 *     term => A searchResult 3.0 format query.
 *     format => xml, json, html or rss
 *     pageidx => the page number of search results. If no value is entered, the default is 1.
 *     pagesize => the number of results per page. If no value is entered, the default is 20.
 *     sort => The field to sort by: 'oid', 'sponsor', 'chair', 'title' or 'when'
 *     sortorder => to sort in ascending or descending order. Options: 'true' for descending order (the default), 'false' for ascending order.
 *   Example:
 *     openleg_search(array('type' => 'meeting', term => 'diaz education', 'format => 'xml'))
 * @param $source
 *   The OpenLeg source site to search. Options are OPENLEG_ROOT for the public site, or OPENLEG_STAGING for the staging server,
 *   where testing is done on new OpenLeg features prior to deployment on the public site.
 * @return
 *   The search result.
 */
function openleg_search($params=array(), $source=OPENLEG_ROOT) 
{
  $params += array(
    'type' => NULL,
    'term' => NULL,
    'format' => 'xml',
    'pageidx' => 1,
    'pageSize' => 20,
    'sort' => NULL, // 'when',
    'sortorder' => NULL, // 'true',

    'startdate' => NULL,
    'enddate' => NULL,
    'status' => NULL,
    'memo' => NULL,
    'sameas' => NULL,
    'sponsor' => NULL,
    'cosponsors' => NULL,
    'committee' => NULL,
    'location' => NULL,

  );
  
  if (!$params['type'] && !$params['term']) {
    return FALSE;
  }
  if ($params['sort']) {
    $params['sortorder'] = ($params['sortorder'] === FALSE || strtolower($params['sortorder']) == 'false') ? 'false' : 'true';
  }
  else {
    $params['sortorder'] = NULL;
  }
  $terms=array();
  foreach ($params as $key => $val) {
    if ($val) {
      $terms[] = "$key=" . urlencode($val);
    }
  }
  $path = '';
  $path .= 'search?' . implode('&', $terms);
  
//  drupal_set_message($source . $path); // for testing with Drupal module
  $data = openleg_curl_request($source . $path);
  return $data;
}

/**
 * Return a field's value for the SimpleXMLElement representation of a meeting.
 * 
 * @param $simplexml
 *   A SimpleXMLElement object.
 * @param $field
 *   The name of the field to be returned.
 * @return
 *   A string, unless the value of $field is 'attendees' or 'bills', in which case
 *   the function returns a SimplXMLElement object.
 */
function openleg_meeting($simplexml, $field = 'id') 
{
  switch ($field) {
    case 'date_time':
    case 'meetingDateTime':
      return (string)$simplexml->attributes()->meetingDateTime;
      break;
    case 'day_of_week':
    case 'meetday':
      return (string)$simplexml->attributes()->meetday;
      break;
    case 'location':
      return (string)$simplexml->attributes()->location;
      break;
    case 'committee_name':
    case 'committeeName':
      return (string)$simplexml->attributes()->committeeName;
      break;
    case 'committee_chair':
    case 'committeeChair':
      return (string)$simplexml->attributes()->committeeChair;
      break;
    case 'attendees':
      return $simplexml->attendees[0];
      break;
    case 'bills':
      return $simplexml->bills[0];
      break;
    case 'note':
    case 'notes':
      return (string)$simplexml->notes;
      break;
    case 'id':
    default:
      return (string)$simplexml->attributes()->id;
      break;
  }
}

/**
 * Return a single attribute from a SimpleXMLElement representation of an OpenLeg object.
 * 
 * @param $simplexml
 *   A SimpleXMLElement object.
 * @param $field
 *   The name of the attribute to be returned.
 * @return
 *   A string.
 */
function openleg_attribute($simplexml, $field = 'name') 
{
  return (string)$simplexml->attributes()->{$field};
}

/**
 * Return a field's value for the SimpleXMLElement representation of a bill.
 * 
 * @param $simplexml
 *   A SimpleXMLElement object.
 * @param $field
 *   The name of the field to be returned.
 * @return
 *   A string.
 */
function openleg_bill($simplexml, $field = 'sponsor') 
{
  switch ($field) {
    case 'year':
      return (string)$simplexml->attributes()->year;
      break;
    case 'id':
      return (string)$simplexml->attributes()->id;
      break;
    case 'current_committee':
    case 'currentCommittee':
      return (string)$simplexml->currentCommittee;
      break;
    case 'law_section':
    case 'lawSection':
      return (string)$simplexml->lawSection;
      break;
    case 'same_as':
    case 'sameAs':
      return (string)$simplexml->sameAs;
      break;
    case 'sponsor':
      return (string)$simplexml->sponsor->attributes()->fullname;
      break;
    case 'summary':
      return (string)$simplexml->summary;
      break;
    case 'title':
      return (string)$simplexml->title;
      break;
  }
}