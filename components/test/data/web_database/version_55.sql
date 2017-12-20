PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE meta(key LONGVARCHAR NOT NULL UNIQUE PRIMARY KEY, value LONGVARCHAR);
INSERT INTO "meta" VALUES('version','55');
INSERT INTO "meta" VALUES('last_compatible_version','55');
INSERT INTO "meta" VALUES('Builtin Keyword Version','70');
INSERT INTO "meta" VALUES('Default Search Provider ID','2');
CREATE TABLE web_intents ( service_url LONGVARCHAR, action VARCHAR, type VARCHAR, title LONGVARCHAR, disposition VARCHAR, scheme VARCHAR, UNIQUE (service_url, action, scheme, type));
CREATE TABLE web_intents_defaults ( action VARCHAR, type VARCHAR, url_pattern LONGVARCHAR, user_date INTEGER, suppression INTEGER, service_url LONGVARCHAR, scheme VARCHAR, UNIQUE (action, scheme, type, url_pattern));
CREATE TABLE keywords (id INTEGER PRIMARY KEY,short_name VARCHAR NOT NULL,keyword VARCHAR NOT NULL,favicon_url VARCHAR NOT NULL,url VARCHAR NOT NULL,safe_for_autoreplace INTEGER,originating_url VARCHAR,date_created INTEGER DEFAULT 0,usage_count INTEGER DEFAULT 0,input_encodings VARCHAR,show_in_default_list INTEGER,suggest_url VARCHAR,prepopulate_id INTEGER DEFAULT 0,created_by_policy INTEGER DEFAULT 0,instant_url VARCHAR,last_modified INTEGER DEFAULT 0,sync_guid VARCHAR,alternate_urls VARCHAR,search_terms_replacement_key VARCHAR,image_url VARCHAR,search_url_post_params VARCHAR,suggest_url_post_params VARCHAR,instant_url_post_params VARCHAR,image_url_post_params VARCHAR,new_tab_url VARCHAR);
CREATE TABLE web_app_icons (url LONGVARCHAR,width int,height int,image BLOB, UNIQUE (url, width, height));
CREATE TABLE web_apps (url LONGVARCHAR UNIQUE,has_all_images INTEGER NOT NULL);
CREATE TABLE autofill (name VARCHAR, value VARCHAR, value_lower VARCHAR, date_created INTEGER DEFAULT 0, date_last_used INTEGER DEFAULT 0, count INTEGER DEFAULT 1, PRIMARY KEY (name, value));
CREATE TABLE credit_cards ( guid VARCHAR PRIMARY KEY, name_on_card VARCHAR, expiration_month INTEGER, expiration_year INTEGER, card_number_encrypted BLOB, date_modified INTEGER NOT NULL DEFAULT 0, origin VARCHAR DEFAULT '');
CREATE TABLE autofill_profiles ( guid VARCHAR PRIMARY KEY, company_name VARCHAR, street_address VARCHAR, dependent_locality VARCHAR, city VARCHAR, state VARCHAR, zipcode VARCHAR, sorting_code VARCHAR, country_code VARCHAR, date_modified INTEGER NOT NULL DEFAULT 0, origin VARCHAR DEFAULT '');
INSERT INTO "autofill_profiles" VALUES('00000000-0000-0000-0000-000000000001','Google Inc','340 Main St','','Los Angeles','CA','90291','','US',1395948829,'Chrome settings');
CREATE TABLE autofill_profile_names ( guid VARCHAR, first_name VARCHAR, middle_name VARCHAR, last_name VARCHAR);
INSERT INTO "autofill_profile_names" VALUES('B41FE6E0-B13E-2A2A-BF0B-29FCE2C3ADBD','Jon','','Smith');
CREATE TABLE autofill_profile_emails ( guid VARCHAR, email VARCHAR);
INSERT INTO "autofill_profile_emails" VALUES('B41FE6E0-B13E-2A2A-BF0B-29FCE2C3ADBD','');
CREATE TABLE autofill_profile_phones ( guid VARCHAR, number VARCHAR);
INSERT INTO "autofill_profile_phones" VALUES('B41FE6E0-B13E-2A2A-BF0B-29FCE2C3ADBD','');
CREATE TABLE autofill_profiles_trash ( guid VARCHAR);
CREATE TABLE token_service (service VARCHAR PRIMARY KEY NOT NULL,encrypted_token BLOB);
CREATE INDEX web_intents_index ON web_intents (action);
CREATE INDEX web_intents_default_index ON web_intents_defaults (action);
CREATE INDEX web_apps_url_index ON web_apps (url);
CREATE INDEX autofill_name ON autofill (name);
CREATE INDEX autofill_name_value_lower ON autofill (name, value_lower);
COMMIT;
