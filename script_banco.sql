-- =============================================================
-- Script de Criação do Banco de Dados
-- Aluno: Herlan
-- Sintaxe: MySQL
-- =============================================================

-- 1. Criar o Database com o nome do aluno
DROP DATABASE IF EXISTS Herlan;
CREATE DATABASE Herlan;
USE Herlan;

-- =============================================================
-- 2. Criação das Tabelas (sem FKs)
-- =============================================================

CREATE TABLE Sexo (
    idSexo INT NOT NULL,
    Descricao VARCHAR(45),
    PRIMARY KEY (idSexo)
);

CREATE TABLE TipoCliente (
    idTipoCliente INT NOT NULL,
    Descricao VARCHAR(45),
    PRIMARY KEY (idTipoCliente)
);

CREATE TABLE PorteDoPet (
    idPorteDoPet INT NOT NULL,
    Descricao VARCHAR(45),
    PRIMARY KEY (idPorteDoPet)
);

CREATE TABLE Cliente (
    idCliente INT NOT NULL,
    Nome VARCHAR(45),
    Nascimento DATETIME,
    TipoCliente_idTipoCliente INT NOT NULL,
    Sexo_idSexo INT NOT NULL,
    PRIMARY KEY (idCliente)
);

CREATE TABLE Telefone (
    Numero VARCHAR(40) NOT NULL,
    Cliente_idCliente INT NOT NULL,
    PRIMARY KEY (Numero, Cliente_idCliente)
);

CREATE TABLE Endereco (
    idEndereco INT NOT NULL,
    Logradouro VARCHAR(45),
    Numero VARCHAR(45),
    Complemento VARCHAR(45),
    Bairro VARCHAR(45),
    Cidade VARCHAR(45),
    CEP VARCHAR(8),
    UF VARCHAR(2),
    Cliente_idCliente INT NOT NULL,
    PRIMARY KEY (idEndereco)
);

CREATE TABLE Raca (
    idRaca INT NOT NULL,
    Nome VARCHAR(45),
    Descricao VARCHAR(45),
    PorteDoPet_idPorteDoPet INT NOT NULL,
    PRIMARY KEY (idRaca)
);

CREATE TABLE Pet (
    idPet INT NOT NULL,
    Nome VARCHAR(45),
    Nascimento DATETIME,
    Cliente_idCliente INT NOT NULL,
    Sexo_idSexo INT NOT NULL,
    Raca_idRaca INT NOT NULL,
    PRIMARY KEY (idPet)
);

-- =============================================================
-- 3. Criação das Foreign Keys (ALTER TABLE)
-- =============================================================

ALTER TABLE Cliente
    ADD CONSTRAINT fk_Cliente_TipoCliente
    FOREIGN KEY (TipoCliente_idTipoCliente) REFERENCES TipoCliente (idTipoCliente);

ALTER TABLE Cliente
    ADD CONSTRAINT fk_Cliente_Sexo
    FOREIGN KEY (Sexo_idSexo) REFERENCES Sexo (idSexo);

ALTER TABLE Telefone
    ADD CONSTRAINT fk_Telefone_Cliente
    FOREIGN KEY (Cliente_idCliente) REFERENCES Cliente (idCliente);

ALTER TABLE Endereco
    ADD CONSTRAINT fk_Endereco_Cliente
    FOREIGN KEY (Cliente_idCliente) REFERENCES Cliente (idCliente);

ALTER TABLE Raca
    ADD CONSTRAINT fk_Raca_PorteDoPet
    FOREIGN KEY (PorteDoPet_idPorteDoPet) REFERENCES PorteDoPet (idPorteDoPet);

ALTER TABLE Pet
    ADD CONSTRAINT fk_Pet_Cliente
    FOREIGN KEY (Cliente_idCliente) REFERENCES Cliente (idCliente);

ALTER TABLE Pet
    ADD CONSTRAINT fk_Pet_Sexo
    FOREIGN KEY (Sexo_idSexo) REFERENCES Sexo (idSexo);

ALTER TABLE Pet
    ADD CONSTRAINT fk_Pet_Raca
    FOREIGN KEY (Raca_idRaca) REFERENCES Raca (idRaca);
