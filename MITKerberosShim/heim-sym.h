/* generated file, no dont edit */
void __attribute__((constructor)) heim_load_frameworks(void);
void heim_load_functions(void);
extern void (*fun_krb5_cc_end_seq_get)();
extern void (*fun_krb5_config_get_string)();
extern void (*fun_krb5_set_default_in_tkt_etypes)();
extern void (*fun_krb5_get_pw_salt)();
extern void (*fun_krb5_free_salt)();
extern void (*fun_krb5_string_to_key_data_salt)();
extern void (*fun_krb5_free_keyblock_contents)();
extern void (*fun_krb5_set_real_time)();
extern void (*fun_krb5_mk_req_extended)();
extern void (*fun_krb5_free_keyblock)();
extern void (*fun_krb5_auth_con_getremotesubkey)();
extern void (*fun_krb5_auth_con_getlocalsubkey)();
extern void (*fun_krb5_set_password)();
extern void (*fun_krb5_set_password_using_ccache)();
extern void (*fun_krb5_realm_compare)();
extern void (*fun_krb5_get_renewed_creds)();
extern void (*fun_krb5_get_validated_creds)();
extern void (*fun_krb5_get_init_creds_keytab)();
extern void (*fun_krb5_prompter_posix)();
extern void (*fun_krb5_string_to_deltat)();
extern void (*fun_krb5_get_all_client_addrs)();
extern void (*fun_krb5_kt_get_type)();
extern void (*fun_krb5_kt_add_entry)();
extern void (*fun_krb5_kt_remove_entry)();
extern void (*fun_krb5_mk_req)();
extern void (*fun_krb5_kt_get_name)();
extern void (*fun_krb5_rd_req)();
extern void (*fun_krb5_free_ticket)();
extern void (*fun_krb5_build_principal_va)();
extern void (*fun_krb5_build_principal_va_ext)();
extern void (*fun_krb5_cc_cache_match)();
extern void (*fun_krb5_cc_close)();
extern void (*fun_krb5_cc_default)();
extern void (*fun_krb5_cc_get_config)();
extern void (*fun_krb5_cc_get_full_name)();
extern void (*fun_krb5_cc_get_name)();
extern void (*fun_krb5_cc_get_principal)();
extern void (*fun_krb5_cc_get_type)();
extern void (*fun_krb5_cc_initialize)();
extern void (*fun_krb5_cc_move)();
extern void (*fun_krb5_cc_new_unique)();
extern void (*fun_krb5_cc_resolve)();
extern void (*fun_krb5_cc_store_cred)();
extern void (*fun_krb5_cc_switch)();
extern void (*fun_krb5_cc_retrieve_cred)();
extern void (*fun_krb5_cc_remove_cred)();
extern void (*fun_krb5_cc_get_kdc_offset)();
extern void (*fun_krb5_cc_set_kdc_offset)();
extern void (*fun_krb5_cc_next_cred)();
extern void (*fun_krb5_cccol_last_change_time)();
extern void (*fun_krb5_crypto_init)();
extern void (*fun_krb5_crypto_getblocksize)();
extern void (*fun_krb5_crypto_destroy)();
extern void (*fun_krb5_decrypt_ivec)();
extern void (*fun_krb5_encrypt_ivec)();
extern void (*fun_krb5_crypto_getenctype)();
extern void (*fun_krb5_generate_random_keyblock)();
extern void (*fun_krb5_get_wrapped_length)();
extern void (*fun_krb5_copy_creds_contents)();
extern void (*fun_krb5_copy_data)();
extern void (*fun_krb5_copy_principal)();
extern void (*fun_krb5_data_copy)();
extern void (*fun_krb5_data_free)();
extern void (*fun_krb5_data_zero)();
extern void (*fun_krb5_free_context)();
extern void (*fun_krb5_free_cred_contents)();
extern void (*fun_krb5_free_creds)();
extern void (*fun_krb5_free_principal)();
extern void (*fun_krb5_sname_to_principal)();
extern void (*fun_krb5_get_credentials)();
extern void (*fun_krb5_get_error_string)();
extern void (*fun_krb5_get_default_principal)();
extern void (*fun_krb5_get_error_message)();
extern void (*fun_krb5_get_init_creds_opt_alloc)();
extern void (*fun_krb5_get_init_creds_opt_free)();
extern void (*fun_krb5_get_init_creds_opt_set_canonicalize)();
extern void (*fun_krb5_get_init_creds_opt_set_forwardable)();
extern void (*fun_krb5_get_init_creds_opt_set_proxiable)();
extern void (*fun_krb5_get_init_creds_opt_set_renew_life)();
extern void (*fun_krb5_get_init_creds_opt_set_tkt_life)();
extern void (*fun_krb5_get_init_creds_password)();
extern void (*fun_krb5_get_kdc_cred)();
extern void (*fun_krb5_get_kdc_sec_offset)();
extern void (*fun_krb5_init_context)();
extern void (*fun_krb5_make_principal)();
extern void (*fun_krb5_parse_name)();
extern void (*fun_krb5_principal_compare)();
extern void (*fun_krb5_principal_get_realm)();
extern void (*fun_krb5_timeofday)();
extern void (*fun_krb5_unparse_name)();
extern void (*fun_krb5_us_timeofday)();
extern void (*fun_krb5_kt_start_seq_get)();
extern void (*fun_krb5_kt_end_seq_get)();
extern void (*fun_krb5_xfree)();
extern void (*fun_krb5_kt_next_entry)();
extern void (*fun_krb5_kt_free_entry)();
extern void (*fun_gsskrb5_extract_authz_data_from_sec_context)();
extern void (*fun_krb5_sendauth)();
extern void (*fun_krb5_free_ap_rep_enc_part)();
extern void (*fun_krb5_free_error)();
extern void (*fun_krb5_recvauth)();
extern void (*fun_krb5_recvauth_match_version)();
extern void (*fun_krb5_mk_priv)();
extern void (*fun_krb5_rd_priv)();
extern void (*fun_krb5_mk_safe)();
extern void (*fun_krb5_rd_safe)();
extern void (*fun_krb5_set_home_dir_access)();
extern void (*fun_krb5_verify_init_creds)();
extern void (*fun_krb5_verify_init_creds_opt_init)();
extern void (*fun_krb5_verify_init_creds_opt_set_ap_req_nofail)();
extern void (*fun_krb5_kuserok)();
extern void (*fun_com_right)();
extern void (*fun_com_right_r)();
extern void (*fun_gss_import_name)();
extern void (*fun_krb5_appdefault_boolean)();
extern void (*fun_krb5_appdefault_string)();
extern void (*fun_gss_accept_sec_context)();
extern void (*fun_gss_acquire_cred)();
extern void (*fun_gss_add_cred)();
extern void (*fun_gss_add_oid_set_member)();
extern void (*fun_gss_canonicalize_name)();
extern void (*fun_gss_compare_name)();
extern void (*fun_gss_context_time)();
extern void (*fun_gss_create_empty_oid_set)();
extern void (*fun_gss_delete_sec_context)();
extern void (*fun_gss_display_name)();
extern void (*fun_gss_display_status)();
extern void (*fun_gss_duplicate_name)();
extern void (*fun_gss_export_name)();
extern void (*fun_gss_export_sec_context)();
extern void (*fun_gss_get_mic)();
extern void (*fun_gss_import_sec_context)();
extern void (*fun_gss_indicate_mechs)();
extern void (*fun_gss_init_sec_context)();
extern void (*fun_gss_inquire_context)();
extern void (*fun_gss_inquire_cred)();
extern void (*fun_gss_inquire_cred_by_mech)();
extern void (*fun_gss_inquire_names_for_mech)();
extern void (*fun_gss_krb5_ccache_name)();
extern void (*fun_gss_krb5_copy_ccache)();
extern void (*fun_gss_krb5_export_lucid_sec_context)();
extern void (*fun_gss_krb5_free_lucid_sec_context)();
extern void (*fun_gss_krb5_set_allowable_enctypes)();
extern void (*fun_gss_oid_equal)();
extern void (*fun_gss_oid_to_str)();
extern void (*fun_gss_process_context_token)();
extern void (*fun_gss_release_buffer)();
extern void (*fun_gss_release_cred)();
extern void (*fun_gss_release_name)();
extern void (*fun_gss_release_oid)();
extern void (*fun_gss_release_oid_set)();
extern void (*fun_gss_seal)();
extern void (*fun_gss_test_oid_set_member)();
extern void (*fun_gss_unseal)();
extern void (*fun_gss_unwrap)();
extern void (*fun_gss_verify_mic)();
extern void (*fun_gss_wrap)();
extern void (*fun_gss_wrap_size_limit)();
extern void (*fun_krb5_cc_start_seq_get)();
extern void (*fun_krb5_cc_default_name)();
extern void (*fun_krb5_cc_destroy)();
extern void (*fun_krb5_cccol_cursor_free)();
extern void (*fun_krb5_cccol_cursor_new)();
extern void (*fun_krb5_cccol_cursor_next)();
extern void (*fun_krb5_free_host_realm)();
extern void (*fun_krb5_get_default_realm)();
extern void (*fun_krb5_get_host_realm)();
extern void (*fun_krb5_gss_register_acceptor_identity)();
extern void (*fun_krb5_cc_set_default_name)();
extern void (*fun_krb5_kt_resolve)();
extern void (*fun_krb5_kt_default)();
extern void (*fun_krb5_kt_default_name)();
extern void (*fun_krb5_kt_close)();
extern void (*fun_krb5_kt_destroy)();
extern void (*fun_krb5_auth_con_free)();
extern void (*fun_krb5_auth_con_init)();
extern void (*fun_krb5_auth_con_genaddrs)();
extern void (*fun_krb5_auth_con_getlocalseqnumber)();
extern void (*fun_krb5_auth_con_getremoteseqnumber)();
extern void (*fun_krb5_auth_con_setflags)();
extern void (*fun_krb5_auth_con_getflags)();
extern void (*fun_krb5_clear_error_message)();
extern void (*fun_krb5_free_error_message)();
extern void (*fun_krb5_set_default_realm)();
extern void (*fun_krb5_set_error_message)();
extern void (*fun_krb5_vset_error_message)();
extern void (*fun_com_err)();
extern void (*fun_com_err_va)();
extern void (*fun_reset_com_err_hook)();
extern void (*fun_set_com_err_hook)();
