<?php

/**
 * @file
 * Handles the uploading of the PDF file as part of the ingest process.
 */

/**
 * Defines a file upload form for uploading the pdf file.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_document_document_upload_form(array $form, array &$form_state) {
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  module_load_include('inc', 'islandora_jodconverter', 'includes/utilities');

  $pdf = islandora_jodconverter_get_format('pdf');
  $pdf['from'][] = 'pdf';
  $extensions = array(implode(' ', $pdf['from']));
  $form = array();
  $form['file'] = array(
    '#title' => t('Document File'),
    '#type' => 'managed_file',
    '#required' => TRUE,
    '#description' => t('Select file to upload.<br/>Files must be less than <strong>@size MB.</strong><br/>Allowed file types: <strong@ext.</strong>', array('@size' => $upload_size, '@ext' => $extensions[0])),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => $extensions,
      // Assume it's specified in MB.
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );

  if (variable_get('islandora_document_allow_text_upload', FALSE)) {
    $form['islandora_document_text_upload'] = array(
      '#type' => 'checkbox',
      '#title' => t("Add text file to this upload?"),
      '#default_value' => FALSE,
    );
    $form['text'] = array(
      '#title' => t('Document text'),
      '#type' => 'managed_file',
      '#required' => FALSE,
      '#description' => t('Select text file to upload.<br/>Files must be less than <strong>@size MB.</strong><br/>Allowed file types: <strong>@ext.</strong><br />This file is optional.', array('@size' => $upload_size, '@ext' => 'txt')),
      '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => array('txt'),
        // Assume it's specified in MB.
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
      '#states' => array(
        'visible' => array(
          ':input[name="islandora_document_text_upload"]' => array('checked' => TRUE),
        ),
      ),
    );
  }
  return $form;
}

/**
 * Adds the uploaded file into the ingestible objects 'OBJ' datastream.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_document_document_upload_form_submit(array $form, array &$form_state) {
  $object = islandora_ingest_form_get_object($form_state);
  if (empty($object['OBJ'])) {
    $ds = $object->constructDatastream('OBJ', 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['OBJ'];
  }
  $file = file_load($form_state['values']['file']);
  $path = drupal_realpath($file->uri);
  $ds->setContentFromFile($path, FALSE);
  $ds->label = $file->filename;
  $ds->mimetype = $file->filemime;

  if (isset($form_state['values']['text']) && ($form_state['values']['text'] > 0)) {
    if (empty($object['FULL_TEXT'])) {
      $ds = $object->constructDatastream('FULL_TEXT', 'M');
      $object->ingestDatastream($ds);
    }
    else {
      $ds = $object['FULL_TEXT'];
    }
    $text_file = file_load($form_state['values']['text']);
    $text_path = drupal_realpath($text_file->uri);
    $ds->setContentFromFile($text_path, FALSE);
    $ds->label = $text_file->filename;
    $ds->mimetype = $text_file->filemime;
  }
}
