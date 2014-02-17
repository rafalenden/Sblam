<?php

/**
 * @file
 * Includes only functions used in administration pages.
 */

/**
 * Configuration page.
 */
function sblam_settings_form() {
  $form = array();

  $form['#attached']['css'][] = drupal_get_path('module', 'sblam') . '/sblam.css';

  // Sblam Configuration.
  $form['configuration'] = array(
    '#type' => 'fieldset',
    '#title' => t('Sblam configuration'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['configuration']['sblam_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#description' => t('Sblam API service key. <a href="!url">Get new API key</a>.', array('!url' => 'http://sblam.com/key.html')),
    '#default_value' => variable_get('sblam_api_key'),
    '#required' => TRUE,
  );
  $form['configuration']['sblam_fallback'] = array(
    '#type' => 'radios',
    '#title' => t('When Sblam server is down or unreachable'),
    '#options' => array(
      0 => t('Block all form submissions'),
      1 => t('Accept all form submissions'),
    //2 => t('Show captcha'),
    ),
    '#default_value' => variable_get('sblam_fallback', 0),
  );
  $form['configuration']['sblam_log'] = array(
    '#type' => 'checkbox',
    '#title' => t('Log blocked form submissions'),
    '#description' => t('Log submissions that are blocked due to Sblam protection.'),
    '#default_value' => variable_get('sblam_log', 0),
  );
  $form['configuration']['sblam_servers'] = array(
    '#type' => 'textarea',
    '#title' => t('Sblam servers'),
    '#description' => t('<a href="@sblam-server-url">Sblam server</a> addresses to connect with.', array('@sblam-server-url' => 'https://github.com/pornel/Sblam')),
    '#default_value' => variable_get('sblam_servers', implode(sblam_default_servers(), "\r\n")),
    '#required' => TRUE,
    '#rows' => 3,
  );

  // Forms protectable by Sblam.
  $protectable_forms = array();

  // User forms.
  $protectable_forms['user'] = array(
    '#title' => t('User related forms'),
    '#forms' => array(
      'user_register_form' => array(
        '#title' => t('User Registration form'),
        '#fields' => array('name', 'mail'),
      ),
      'user_pass' => array(
        '#title' => t('User Password Reset form'),
        '#fields' => array('name'),
      ),
    ),
  );

  // Webforms.
  if (module_exists('webform')) {
    $protectable_forms['webform'] = array(
      '#title' => t('Webforms'),
      '#forms' => array(
        'webform_1' => array(
          '#title' => t('User Registration form'),
          '#fields' => array('name' => t('Username'), 'mail' => t('Email')),
        ),
        'webform_2' => array(
          '#title' => t('User Password Reset form'),
          '#fields' => array('name' => t('Username')),
        ),
      ),
    );
    $query = db_select('webform', 'w');
    $query->join('node', 'n', 'w.nid = n.nid');
    $query->fields('n');
    $webform_nodes = $query->execute()->fetchAllAssoc('nid');
    foreach ($webform_nodes as $nid => $node) {
      $components = db_query("SELECT form_key, name, type FROM {webform_component}
        WHERE nid = :nid AND type IN(:types)", array(':nid' => $nid, ':types' => array('textfield', 'textarea', 'number', 'email')))->fetchAllKeyed(0, 1);
      $protectable_forms['webform']['#forms']['webform_client_form_' . $nid] = array(
        '#title' => $node->title,
        '#fields' => $components,
      );
    }
  }

  // Contact forms.
  if (module_exists('contact')) {
    $protectable_forms['contact'] = array(
      '#title' => t('Contact forms'),
      '#forms' => array(
        'contact_site_form' => array(
          '#title' => t('Contact site form'),
          '#fields' => array(
            'name' => t('Your name'),
            'mail' => t('Your e-mail address'),
            'subject' => t('Subject'),
            'message' => t('Message'),
          ),
        ),
        'contact_personal_form' => array(
          '#title' => t('Contact personal form'),
          '#fields' => array(
            'name' => t('Your name'),
            'mail' => t('Your e-mail address'),
            'subject' => t('Subject'),
            'message' => t('Message'),
          ),
        ),
      ),
    );
  }
  // if (module_exists('profile')) {}

  // Node forms.
  $protectable_forms['user'] = array(
    '#title' => t('User forms'),
    '#forms' => array(
      'user_register_form' => array(
        '#title' => t('User Registration form'),
        '#fields' => array('name' => t('Username'), 'mail' => t('Mail')),
      ),
      'user_pass' => array(
        '#title' => t('User Password Reset form'),
        '#fields' => array('name' => t('Username')),
      ),
    ),
  );
  $types = node_type_get_types();

  if (!empty($types)) {
    $protectable_forms['node'] = array(
      '#title' => t('Node forms'),
      '#forms' => array(),
    );
    foreach ($types as $type) {
      $id = $type->type . '_node_form';
      $protectable_forms['node']['#forms'][$id] = array(
        '#title' => t('@name node form', array('@name' => $type->name)),
        '#fields' => array('title' => t('Title')),
      );
      $fields = field_info_instances('node', $type->type);
      foreach ($fields as $field_name => $field) {
        if (in_array($field['widget']['module'], array('text', 'number', 'email', 'link', 'telephone'))) {
          $protectable_forms['node']['#forms'][$id]['#fields'][$field_name] = $field['label'];
        }
      }
    }

    // Comment forms.
    if (module_exists('comment')) {
      $protectable_forms['comment'] = array(
        '#title' => t('Comment forms'),
        '#forms' => array(),
      );
      foreach ($types as $type) {
        $id = 'comment_node_' . $type->type . '_form';
        $protectable_forms['comment']['#forms'][$id] = array(
          '#title' => t('@name comment form', array('@name' => $type->name)),
        );
        if (variable_get('comment_subject_field_' . $type->type, 1)) {
          $protectable_forms['comment']['#forms'][$id]['#fields']['subject'] = t('Subject');
        }
        $fields = field_info_instances('comment', 'comment_node_' . $type->type);
        foreach ($fields as $field_name => $field) {
          $protectable_forms['comment']['#forms'][$id]['#fields'][$field_name] = $field['label'];
        }
      }
    }
  }

  // Forms protected by Sblam.
  $protected_forms = variable_get('sblam_protected_forms');

  $form['sblam_protected_forms'] = array(
    '#type' => 'fieldset',
    '#title' => t('Forms protected by Sblam'),
    '#description' => t("Check the boxes next to individual forms on which you'd like Sblam protection enabled."),
    '#collapsible' => TRUE,
    '#collapsed' => !empty($protected_forms),
    '#tree' => TRUE,
  );

  // Build tree of checkboxes.
  foreach ($protectable_forms as $group_key => $group) {
    $form['sblam_protected_forms'][$group_key] = array(
      '#markup' => '<h3 class="sblam-group-title-' . $group_key . '">' . $group['#title'] . '</h3>',
    );
    foreach ($group['#forms'] as $form_key => $protectable_form) {
      $form['sblam_protected_forms'][$form_key]['enabled'] = array(
        '#title' => $protectable_form['#title'],
        '#type' => 'checkbox',
        '#attributes' => array('class' => array('sblam-form')),
        '#default_value' => isset($protected_forms[$form_key]['enabled']) ? $protected_forms[$form_key]['enabled']
           : 0,
      );
      if (!empty($protectable_form['#fields'])) {
        $form['sblam_protected_forms'][$form_key]['fields'] = array(
          '#title' => t('Fields'),
          '#type' => 'checkboxes',
          '#options' => $protectable_form['#fields'],
          '#default_value' => isset($protected_forms[$form_key]['fields']) ? $protected_forms[$form_key]['fields']
             : array(),
          '#states' => array(
            'visible' => array(
              ':input[name="sblam_protected_forms[' . $form_key . '][enabled]"]' => array('checked' => TRUE),
            ),
          ),
        );
      }
    }
  }

  $form['#submit'][] = 'sblam_settings_form_submit';

  return system_settings_form($form);
}

function sblam_settings_form_submit($form, &$form_state) {
  $form_state['values']['sblam_servers'] = trim($form_state['values']['sblam_servers']);
}
