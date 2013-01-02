<?php
/**
 * @file
 * Simple panels content type for displaying an uploaded image and linking it to a provided url/path
 */
/** @noinspection PhpUnusedLocalVariableInspection */
$plugin = array(
  'title'           => t('Burgmann Image Link'),
  'content_types'   => 'burg_panels_image_link',
  'single'          => FALSE,
  'render callback' => 'burg_panels_image_link_render',
  'description'     => t('Displays an uploaded image and linking it to a provided url/path.'),
  'edit form'       => 'burg_panels_image_link_edit_form',
  'admin title'     => 'burg_panels_image_link_admin_title',
  'admin info'      => 'burg_panels_image_link_admin_info',
  'category'        => array(
    'miscellaneous',
    0
  ),
);
/**
 * Title callback for admin page
 */
function burg_panels_image_link_admin_title($subtype, $conf, $context = NULL) {
  return t('Image Link');
}

/**
 * Callback to provide administrative info (the preview in panels when building a panel).
 */
function burg_panels_image_link_admin_info($subtype, $conf, $context = NULL) {
  $block = new stdClass();
  $block->title = t('Image Link');
  $config = array();
  $config[] = t('Title') . ': ' . ( isset( $conf['title'] ) && !empty( $conf['title'] ) ? filter_xss_admin($conf['title']) : t('Not Set') );
  $config[] = t('Target') . ': <b>' . ( isset( $conf['target_url'] ) && !empty( $conf['target_url'] ) ? htmlentities($conf['target_url']) : t('Not Set') ) . '</b>';
  $config[] = t('Caption') . ': ' . ( isset( $conf['caption'] ) && !empty( $conf['caption'] ) ? filter_xss_admin($conf['caption']) : t('Not Set') );
  $config[] = t('Filename') . ': <b>' . ( isset( $conf['image'] ) ? drupal_realpath('public://' . $conf['image']) : t('Not Set') ) . '</b>';
  $block->content = theme_item_list(array( 'items' => $config, 'title' => NULL, 'type' => 'ul', 'attributes' => array() ));
  return $block;
}

/**
 * Run-time rendering of the body of the block (content type)
 *
 * @param $subtype
 * @param $conf
 *   Configuration as done at admin time
 * @param $args
 * @param $context
 *   Context - in this case we don't have any
 *
 * @return
 *   An object with at least title and content members
 */
function burg_panels_image_link_render($subtype, $conf, $args, $context) {
  if ( isset( $conf['image'] ) ) {
    if ( isset( $conf['title'] ) && !empty( $conf['title'] ) ) {
      if ( isset( $conf['target_url'] ) && !empty( $conf['target_url'] ) ) {
        $content = '<div class="panels-image-link-title">' . l(filter_xss_admin($conf['title']), $conf['target_url'], array( 'html' => TRUE, )) . '</div>';
      } else {
        $content = '<div class="panels-image-link-title">' . filter_xss_admin($conf['title']) . '</div>';
      }
    }
    $image_vars = array(
      'path'       => file_create_url('public://' . $conf['image']),
      'alt'        => NULL,
      'title'      => NULL,
      'attributes' => array(),
    );
    $image = theme_image($image_vars);
    if ( isset( $conf['target_url'] ) && !empty( $conf['target_url'] ) ) {
      $content .= l($image, $conf['target_url'], array( 'html' => TRUE, ));
    }
    else {
      $content .= $image;
    }
    if ( isset( $conf['caption'] ) && !empty( $conf['caption'] ) ) {
      $content .= '<div class="panels-image-link-caption">' . filter_xss_admin($conf['caption']) . '</div>';
    }
    $block = new stdClass();
    $block->title = NULL;
    $block->content = $content;
    return $block;
  }
  return NULL;
}

/**
 * Edit callback for the content type.
 */
function burg_panels_image_link_edit_form($form, &$form_state) {
  //Disable title override
  unset( $form['override_title'] );
  unset( $form['override_title_markup'] );
  unset( $form['override_title_text'] );
  $conf = $form_state['conf'];
/*BURG*/  
  $form['title'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Title'),
    '#description'   => t('Leave blank to prevent a title from being rendered above the image.'),
    '#size'          => 100,
    '#default_value' => isset( $conf['title'] ) ? $conf['title'] : '',
  );
//
  $form['target_url'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Link Target'),
    '#description'   => t('Leave blank to prevent the image from being rendered as a link.'),
    '#default_value' => isset( $conf['target_url'] ) ? $conf['target_url'] : '',
  );
  $form['caption'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Caption'),
    '#description'   => t('Leave blank to prevent a caption from being rendered below the image.'),
    '#size'          => 500,
    '#default_value' => isset( $conf['caption'] ) ? $conf['caption'] : '',
  );
  $form['image'] = array(
    '#type'          => 'file',
    '#title'         => t('Image'),
    '#description'   => isset( $conf['image'] ) ? t('Currently set as:') . ' ' . drupal_realpath('public://' . $conf['image']) : '',
    '#default_value' => isset( $conf['image'] ) ? $conf['image'] : '',
  );
  return $form;
}

/**
 * Validation callback for the content type.
 */
function burg_panels_image_link_edit_form_validate($form, &$form_state) {
  $file_validators = array(
    'file_validate_is_image' => array(), // Validates file is really an image.
  );
  $file = file_save_upload('image', $file_validators);
  // If the file passed validation:
  if ( $file ) {
    // Move the file, into the Drupal file system
    if ( $file = file_move($file, 'public://') ) {
      // Save the file for use in the submit handler.
      $form_state['storage']['image'] = $file;
    }
    else {
      form_set_error('image', t('Failed to write the uploaded file the site\'s file folder.'));
    }
  }
}

/**
 * Submit callback for settings form
 */
function burg_panels_image_link_edit_form_submit($form, &$form_state) {
  if ( isset( $form_state['storage']['image'] ) ) {
    $file = $form_state['storage']['image'];
    // We are done with the file, remove it from storage.
    unset( $form_state['storage']['image'] );
    // Make the storage of the file permanent
    $file->status = FILE_STATUS_PERMANENT;
    // Save file status.
    file_save($file);
    $form_state['values']['image'] = $file->filename;
  }
  foreach (element_children($form) as $key) {
    if ( isset( $form_state['values'][$key] ) ) {
      $form_state['conf'][$key] = $form_state['values'][$key];
    }
  }
}
