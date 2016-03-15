<?php

class Page {

  # instance variables
  var $pathToWebRoot = "";
  var $pageTitle = "";
  var $pageIdentifier= "";
  var $navIdentifier= "";
  var $styleSheet= "";
  var $inlineStyles = "";
  var $inlineScripts = "";
  var $bodyAttributes = "";

  # set functions
  function setPathToWebRoot($pathToWebRoot) {
    $this->pathToWebRoot = $pathToWebRoot; }

  function setPageTitle($pageTitle) {
    $this->pageTitle = $pageTitle; }

  function setPageIdentifier($pageIdentifier) {
    $this->pageIdentifier = $pageIdentifier; }

  function setNavIdentifier($navIdentifier) {
    $this->navIdentifier = $navIdentifier; }

  function setStyleSheet($styleSheet) {
    $this->styleSheet= $styleSheet; }

  function setInlineStyles($inlineStyles) {
    $this->inlineStyles = $inlineStyles; }

  function setInlineScripts($inlineScripts) {
    $this->inlineScripts = $inlineScripts; }

  function setBodyAttributes($bodyAttributes) {
    $this->bodyAttributes = $bodyAttributes; }

  # get functions
  function getPathToWebRoot() {
    return $this->pathToWebRoot; }

  function getPageTitle() {
    return $this->pageTitle; }

  function getPageIdentifier() {
    return $this->pageIdentifier;
  }

  function getNavIdentifier() {
    return $this->navIdentifier;
  }

  function getNav() {
    $navString = $this->getPathToWebRoot();
    $navString .= 'common/nav_';
    $navString .= $this->navIdentifier;
    $navString .= '.html';
    return $navString;
  }

  function getStyleSheet() {
    return $this->styleSheet; }

  function getInlineStyles() {
    return $this->inlineStyles; }

  function getInlineScripts() {
    return $this->inlineScripts; }

  function getBodyAttributes() {
    return $this->bodyAttributes; }

} # Page

?>

