﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Testconnect : MonoBehaviour {

    private CommunicateWithArduino Uno;

    // Use this for initialization
    void Start()
    {
        Uno = new CommunicateWithArduino("COM5");
    }
	
	// Update is called once per frame
	void Update () {
		
	}
}