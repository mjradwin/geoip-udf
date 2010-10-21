-- MySQL User Defined Functions for MaxMind GeoIP API
--
-- Copyright (c) 2010, Anchor Intelligence. All rights reserved.
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions
-- are met:
--
-- * Redistributions of source code must retain the above copyright
--   notice, this list of conditions and the following disclaimer.
--
-- * Redistributions in binary form must reproduce the above copyright
--   notice, this list of conditions and the following disclaimer in the
--   documentation and/or other materials provided with the
--   distribution.
--
-- * Neither the name of Anchor Intelligence nor the names of its
--   contributors may be used to endorse or promote products derived
--   from this software without specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
-- "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
-- LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
-- A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
-- OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
-- SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
-- LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
-- DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
-- THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
-- OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

DROP FUNCTION IF EXISTS geoip_country_code;
DROP FUNCTION IF EXISTS geoip_country_name;
DROP FUNCTION IF EXISTS geoip_region;
DROP FUNCTION IF EXISTS geoip_city;
DROP FUNCTION IF EXISTS geoip_dma_code;
DROP FUNCTION IF EXISTS geoip_area_code;
DROP FUNCTION IF EXISTS geoip_latitude;
DROP FUNCTION IF EXISTS geoip_longitude;
DROP FUNCTION IF EXISTS geoip_postal_code;
DROP FUNCTION IF EXISTS geoip_org;
DROP FUNCTION IF EXISTS geoip_isp;

CREATE FUNCTION geoip_country_code RETURNS STRING SONAME 'udf_geoip.so';
CREATE FUNCTION geoip_country_name RETURNS STRING SONAME 'udf_geoip.so';
CREATE FUNCTION geoip_region RETURNS STRING SONAME 'udf_geoip.so';
CREATE FUNCTION geoip_city RETURNS STRING SONAME 'udf_geoip.so';
CREATE FUNCTION geoip_dma_code RETURNS INTEGER SONAME 'udf_geoip.so';
CREATE FUNCTION geoip_area_code RETURNS INTEGER SONAME 'udf_geoip.so';
CREATE FUNCTION geoip_latitude RETURNS REAL SONAME 'udf_geoip.so';
CREATE FUNCTION geoip_longitude RETURNS REAL SONAME 'udf_geoip.so';
CREATE FUNCTION geoip_postal_code RETURNS STRING SONAME 'udf_geoip.so';
CREATE FUNCTION geoip_org RETURNS STRING SONAME 'udf_geoip.so';
CREATE FUNCTION geoip_isp RETURNS STRING SONAME 'udf_geoip.so';
