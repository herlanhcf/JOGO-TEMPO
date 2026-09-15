-- =============================================================
-- Script de Criacao do Banco de Dados
-- Aluno: Herlan
-- Sintaxe: MySQL
-- =============================================================

-- 1. Criar o Database com o nome do aluno
CREATE DATABASE Herlan;
USE Herlan;

-- =============================================================
-- 2. Criacao das Tabelas (sem PKs e sem FKs)
-- =============================================================

-- Tabela: Sexo
CREATE TABLE Sexo (
    idSexo INT NOT NULL AUTO_INCREMENT,
    Descricao VARCHAR(45)
);

-- Tabela: TipoCliente
CREATE TABLE TipoCliente (
    idTipoCliente INT NOT NULL AUTO_INCREMENT,
    Descricao VARCHAR(45)
);

-- Tabela: PorteDoPet
CREATE TABLE PorteDoPet (
    idPorteDoPet INT NOT NULL AUTO_INCREMENT,
    Descricao VARCHAR(45)
);

-- Tabela: Cliente
CREATE TABLE Cliente (
    idCliente INT NOT NULL AUTO_INCREMENT,
    Nome VARCHAR(45),
    Nascimento DATETIME,
    TipoCliente_idTipoCliente INT NOT NULL,
    Sexo_idSexo INT NOT NULL
);

-- Tabela: Telefone
CREATE TABLE Telefone (
    Numero VARCHAR(40) NOT NULL,
    Cliente_idCliente INT NOT NULL
);

-- Tabela: Endereco
CREATE TABLE Endereco (
    idEndereco INT NOT NULL AUTO_INCREMENT,
    Logradouro VARCHAR(45),
    Numero VARCHAR(45),
    Complemento VARCHAR(45),
    Bairro VARCHAR(45),
    Cidade VARCHAR(45),
    CEP VARCHAR(8),
    UF VARCHAR(2),
    Cliente_idCliente INT NOT NULL
);

-- Tabela: Raca
CREATE TABLE Raca (
    idRaca INT NOT NULL AUTO_INCREMENT,
    Nome VARCHAR(45),
    Descricao VARCHAR(45),
    PorteDoPet_idPorteDoPet INT NOT NULL
);

-- Tabela: Pet
CREATE TABLE Pet (
    idPet INT NOT NULL AUTO_INCREMENT,
    Nome VARCHAR(45),
    Nascimento DATETIME,
    Cliente_idCliente INT NOT NULL,
    Sexo_idSexo INT NOT NULL,
    Raca_idRaca INT NOT NULL
);

-- =============================================================
-- 3. Criacao das PKs e FKs via ALTER TABLE
-- =============================================================

-- PKs
ALTER TABLE Sexo
ADD CONSTRAINT PK_Sexo PRIMARY KEY(idSexo);

ALTER TABLE TipoCliente
ADD CONSTRAINT PK_TipoCliente PRIMARY KEY(idTipoCliente);

ALTER TABLE PorteDoPet
ADD CONSTRAINT PK_PorteDoPet PRIMARY KEY(idPorteDoPet);

ALTER TABLE Cliente
ADD CONSTRAINT PK_Cliente PRIMARY KEY(idCliente);

ALTER TABLE Telefone
ADD CONSTRAINT PK_Telefone PRIMARY KEY(Numero, Cliente_idCliente);

ALTER TABLE Endereco
ADD CONSTRAINT PK_Endereco PRIMARY KEY(idEndereco);

ALTER TABLE Raca
ADD CONSTRAINT PK_Raca PRIMARY KEY(idRaca);

ALTER TABLE Pet
ADD CONSTRAINT PK_Pet PRIMARY KEY(idPet);

-- FKs
ALTER TABLE Cliente
ADD CONSTRAINT FK_Cliente_TipoCliente
FOREIGN KEY(TipoCliente_idTipoCliente) REFERENCES TipoCliente(idTipoCliente);

ALTER TABLE Cliente
ADD CONSTRAINT FK_Cliente_Sexo
FOREIGN KEY(Sexo_idSexo) REFERENCES Sexo(idSexo);

ALTER TABLE Telefone
ADD CONSTRAINT FK_Telefone_Cliente
FOREIGN KEY(Cliente_idCliente) REFERENCES Cliente(idCliente);

ALTER TABLE Endereco
ADD CONSTRAINT FK_Endereco_Cliente
FOREIGN KEY(Cliente_idCliente) REFERENCES Cliente(idCliente);

ALTER TABLE Raca
ADD CONSTRAINT FK_Raca_PorteDoPet
FOREIGN KEY(PorteDoPet_idPorteDoPet) REFERENCES PorteDoPet(idPorteDoPet);

ALTER TABLE Pet
ADD CONSTRAINT FK_Pet_Cliente
FOREIGN KEY(Cliente_idCliente) REFERENCES Cliente(idCliente);

ALTER TABLE Pet
ADD CONSTRAINT FK_Pet_Sexo
FOREIGN KEY(Sexo_idSexo) REFERENCES Sexo(idSexo);

ALTER TABLE Pet
ADD CONSTRAINT FK_Pet_Raca
FOREIGN KEY(Raca_idRaca) REFERENCES Raca(idRaca);
