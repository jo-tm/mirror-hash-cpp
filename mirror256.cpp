#include "mirror256.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <vector>
#include <string>
#include <random>

Mirror256::Mirror256(const std::string &m, uint32_t depth, uint32_t size, bool useStandardState)
    : depth(depth)
{
    if (useStandardState)
    {
        initStandardState();
    } else {
        initLastHashes();
    }
    if (!m.empty())
    {
        update(m);
    }
}


void Mirror256::update(const std::string &m) {
    if (m.empty()) {
        return;
    }

    _buffer += m;
    _counter += m.length();

    std::vector<uint8_t> hm;
    while (_buffer.length() >= 32) {
        hm = _mirror256_process(_buffer.substr(0, 32));
        lastHashes.insert(lastHashes.begin(), hm);
        lastHashes.resize(depth);
        _buffer = _buffer.substr(32);
    }

    if (_buffer.length() > 0 && _buffer.length() < 32) {
        hm = _mirror256_process(_buffer + std::string(32 - _buffer.length(), '\x55'));
        _buffer = std::string("");
    }
    _hashed = hm;
}


std::string Mirror256::digest()
{
    return pack(_hashed);
}

std::string Mirror256::hexdigest()
{
    std::string digestStr = digest();
    std::stringstream ss;
    for (char c : digestStr)
    {
        ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(static_cast<uint8_t>(c));
    }
    return ss.str();
}

std::vector<uint8_t> Mirror256::unpack(const std::string &m) {
    std::vector<uint8_t> unpacked;
    for (char c : m) {
        unpacked.push_back(static_cast<uint8_t>(c));
    }
    return unpacked;
}

std::string Mirror256::pack(const std::vector<uint8_t> &hm) {
    std::string packed;
    for (uint8_t byte : hm) {
        packed.push_back(static_cast<char>(byte));
    }
    return packed;
}

std::vector<uint8_t> cubic_root_array(uint64_t jprimerep) {
    std::stringstream ss;
    ss << std::hex << jprimerep;
    std::string hexString = ss.str();
    while (hexString.length() < 12) {
        hexString = "0" + hexString;
    }

    std::vector<uint8_t> ret(8);
    for (size_t i = 0; i < 8; i++) {
        ret[i] = std::stoi(hexString.substr(i, 1), nullptr, 16);
    }

    return ret;
}


void Mirror256::initStandardState() {
    while (lastHashes.size() < depth) {
        size_t i = lastHashes.size();
        std::vector<uint8_t> layer;
        for (size_t j = 8 * i; j < 8 * (i + 1); j++) {
            uint64_t jprimerep = firstPrimesCubicRootDecRep[i];
            std::vector<uint8_t> cubicRootResult = cubic_root_array(jprimerep);
            layer.insert(layer.end(), cubicRootResult.begin(), cubicRootResult.end());
        }
        lastHashes.push_back(layer);
    }
}


void Mirror256::initLastHashes() {
    size_t i = 0;
    while (lastHashes.size() < depth) {
        std::vector<uint8_t> layer;
        for (size_t j = 8 * i; j < 8 * (i + 1); ++j) {
            std::vector<uint8_t> someRandomHash = randomHash();
            layer.insert(layer.end(), someRandomHash.begin(), someRandomHash.end());
        }
        lastHashes.push_back(layer);
        ++i;
    }
}

std::vector<uint8_t> Mirror256::randomHash() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    std::vector<uint8_t> hash(32);
    for (size_t i = 0; i < 32; ++i) {
        hash[i] = static_cast<uint8_t>(dis(gen));
    }
    return hash;
}

std::vector<uint8_t> Mirror256::_mirror256_process(const std::string &m) {
    std::vector<uint8_t> unpacked_m = unpack(m); // Assuming you have already implemented the 'unpack' function

    for (uint32_t layer = 0; layer < depth; ++layer) { // Assuming 'depth' is a pre-defined constant or class variable
        unpacked_m = hashLayerPass(layer, unpacked_m);
    }

    return unpacked_m;
}

/*
#include <iostream>
std::vector<uint8_t> Mirror256::hashLayerPass(uint32_t layer, const std::vector<uint8_t> &block) {
    std::vector<uint8_t> newBlock(block);

    const std::vector<uint8_t>& layerHash = lastHashes[layer];

    std::cout << "layer: " << layer << std::endl;
    std::cout << "layerHash: ";
    for (auto h : layerHash) {
        std::cout << std::hex << (int)h << " ";
    }
    std::cout << std::endl;

    std::cout << "newBlock SIZE " << newBlock.size() << '\n';
    // XOR with layer encoding to avoid 0 to 0 hashes
    for (uint32_t gateIndex = 0; gateIndex < newBlock.size(); ++gateIndex) {
        newBlock[gateIndex] ^= layerHash[gateIndex];
    }

    std::cout << "layerHash SIZE " << layerHash.size() << '\n';
    for (uint32_t gateIndex = 0; gateIndex < DEFAULT_SIZE / 4; ++gateIndex) {
        uint32_t gateType = layerHash[gateIndex] & 0x3;

        uint32_t gateName, gateSymmetry;
        if (gateType % 2 == 0) { // Toffoli
            gateName = 0;
        } else { // Fredkin
            gateName = 1;
        }
        if (gateType >> 1 == 0) { // Regular
            gateSymmetry = 0;
        } else { // Mirrored
            gateSymmetry = 1;
        }

        //std::cout << "gateIndex: " << gateIndex << ", gateType: " << gateType << ", gateName: " << gateName << ", gateSymmetry: " << gateSymmetry << std::endl;

        newBlock = applyGate(gateIndex, gateName, gateSymmetry, newBlock, true);
    }

    for (uint32_t gateIndex = 0; gateIndex < DEFAULT_SIZE / 4; ++gateIndex) {
        uint32_t gateType = layerHash[gateIndex] & 0xc;

        uint32_t gateName, gateSymmetry;
        if ((gateType >> 2) % 2 == 0) { // Toffoli
            gateName = 0;
        } else { // Fredkin
            gateName = 1;
        }
        if ((gateType >> 2) >> 1 == 0) { // Regular
            gateSymmetry = 0;
        } else { // Mirrored
            gateSymmetry = 1;
        }

        //std::cout << "gateIndex: " << gateIndex << ", gateType: " << gateType << ", gateName: " << gateName << ", gateSymmetry: " << gateSymmetry << std::endl;

        newBlock = applyGate(gateIndex, gateName, gateSymmetry, newBlock, false);
    }

    std::cout << "newBlock: ";
    for (auto b : newBlock) {
        std::cout << std::hex << (int)b << " ";
    }
    std::cout << std::endl;

    return newBlock;
}
*/


std::vector<uint8_t> Mirror256::hashLayerPass(uint32_t layer, const std::vector<uint8_t> &block) {
    std::vector<uint8_t> newBlock(block);

    const std::vector<uint8_t>& layerHash = lastHashes[layer];

    // XOR with layer encoding to avoid 0 to 0 hashes
    for (uint32_t gateIndex = 0; gateIndex < newBlock.size(); ++gateIndex) {
        newBlock[gateIndex] ^= layerHash[gateIndex];
    }

    for (uint32_t gateIndex = 0; gateIndex < DEFAULT_SIZE / 4; ++gateIndex) {
        uint32_t gateType = layerHash[gateIndex] & 0x3;

        uint32_t gateName, gateSymmetry;
        if (gateType % 2 == 0) { // Toffoli
            gateName = 0;
        } else { // Fredkin
            gateName = 1;
        }
        if (gateType >> 1 == 0) { // Regular
            gateSymmetry = 0;
        } else { // Mirrored
            gateSymmetry = 1;
        }

        newBlock = applyGate(gateIndex, gateName, gateSymmetry, newBlock, true);
    }

    for (uint32_t gateIndex = 0; gateIndex < DEFAULT_SIZE / 4; ++gateIndex) {
        uint32_t gateType = layerHash[gateIndex] & 0xc;

        uint32_t gateName, gateSymmetry;
        if ((gateType >> 2) % 2 == 0) { // Toffoli
            gateName = 0;
        } else { // Fredkin
            gateName = 1;
        }
        if ((gateType >> 2) >> 1 == 0) { // Regular
            gateSymmetry = 0;
        } else { // Mirrored
            gateSymmetry = 1;
        }

        newBlock = applyGate(gateIndex, gateName, gateSymmetry, newBlock, false);
    }

    return newBlock;
}



uint32_t Mirror256::getWire(uint32_t gateIndex, bool firstSublayer, uint32_t offset) {
    uint32_t wire;
    if (firstSublayer) {
        wire = (gateIndex * 2) + offset;
    } else {
        wire = (gateIndex * 2) + 1 + offset;
    }
    return wire;
}

uint8_t Mirror256::getBit(const std::vector<uint8_t> &block, uint32_t wire) {
    uint32_t byteIndex = wire / 8;
    uint32_t bitIndex = wire % 8;
    return (block[byteIndex] >> bitIndex) & 1;
}

void Mirror256::setBit(std::vector<uint8_t> &block, uint32_t wire, uint8_t bit) {
    uint32_t byteIndex = wire / 8;
    uint32_t bitIndex = wire % 8;
    block[byteIndex] = (block[byteIndex] & ~(1 << bitIndex)) | (bit << bitIndex);
}

std::vector<uint8_t> Mirror256::applyGate(
    uint32_t gateIndex, uint32_t gateName, uint32_t gateSymmetry, const std::vector<uint8_t> &block, bool firstSublayer) {
    std::vector<uint8_t> newBlock(block);
    uint32_t inputWire1 = getWire(gateIndex, firstSublayer);
    uint32_t inputWire2 = getWire(gateIndex, firstSublayer, 1);
    uint32_t outputWire = getWire(gateIndex, !firstSublayer);

    uint8_t inputBit1 = getBit(block, inputWire1);
    uint8_t inputBit2 = getBit(block, inputWire2);
    uint8_t outputBit;

    if (gateName == 0) {
        outputBit = inputBit1 ^ inputBit2;
    } else if (gateName == 1) {
        outputBit = inputBit1 & inputBit2;
    } else if (gateName == 2) {
        outputBit = inputBit1 | inputBit2;
    } else {
        outputBit = inputBit1 ^ (inputBit2 ^ gateSymmetry);
    }

    setBit(newBlock, outputWire, outputBit);
    return newBlock;
}

