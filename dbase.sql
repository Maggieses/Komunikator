CREATE OR REPLACE DATABASE Komunikator;
CREATE OR REPLACE USER 'handler'@'localhost' IDENTIFIED BY '1234';
GRANT ALL PRIVILEGES ON Komunikator.* TO 'handler'@'localhost' IDENTIFIED BY '1234';
USE Komunikator;
CREATE TABLE Users (id INT NOT NULL PRIMARY KEY AUTO_INCREMENT, login varchar(60), hash varchar(32));
CREATE TABLE Messages (id INT NOT NULL PRIMARY KEY AUTO_INCREMENT, target varchar(60), source varchar(60), msg TEXT);
