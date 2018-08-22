﻿using System.Collections;
using System.Collections.Generic;
using System.Threading;
using UnityEngine;

public class DiverControll : MonoBehaviour {
    //Diver Body
    public Transform driverLeftHand;
    public Transform driverRightHand;
    public Transform driver;
    public Transform directionContorl;
    public Transform DistanceCalculator;
    private Vector3 LlastPos = Vector3.zero;//左手上一次的位置
    private Vector3 RlastPos = Vector3.zero;//右手上一次的位置
    private Vector3 Lvector = Vector3.zero;//左手向量
    private Vector3 Rvector = Vector3.zero;//右手向量
    private float Ldistance;//左手距離後面的有多遠的值
    private float Rdistance;//右手距離後面的有多遠的值
    private float LLastDistance;//左手距離後面的有多遠上一次的值
    private float RLastDistance;//右手距離後面的有多遠上一次的值
    private Vector3 diveDirection = Vector3.zero; //身體的方向

    //Fishflock
    public FishFlock.FishFlockController fishflockFlowControl;
    public Transform fishflockOn;
    public Transform fishflockOff;

    //旋轉角度 移動offset 向量和 啟動bool
    private Vector3 LRvector = Vector3.zero; // 兩向量相加
    private float body_vector_angle;
    private float offset = 0;
    private Vector3 rotateValue = Vector3.zero;
    private bool isStarting = false;

    //Motor 參數
    private float waitingTime = 0.5f;
    private int Left_degreeConvertToRotaryCoder(int degree) { return (degree * 1024 / 360); }
    private int Right_degreeConvertToRotaryCoder(int degree) { return (degree * 824 / 360); }
    private struct angle
    {
        public int leftAngle;
        public int rightAngle;
    };
    private angle i;

    //Shark
    public Transform shark;
    public Transform sharkShow;
    private Animator _sharkAnimator;
    private int randomTurn = 0;
    private static bool isShark = false;

    //For test
    private bool specialEffectOn;

    // Use this for initialization
    void Start ()
    {
        //Driver body, right hand, left hand, face, start game
        this.transform.position = new Vector3((driverLeftHand.position.x + driverRightHand.position.x) / 2,
            0.47f, (driverLeftHand.position.z + driverRightHand.position.z) / 2);
        LlastPos = driverLeftHand.position;
        RlastPos = driverRightHand.position;
        diveDirection = directionContorl.position - driver.position;
        UnityEngine.Random.InitState(1337);
        _sharkAnimator = shark.GetComponent<Animator>();
        _sharkAnimator.SetInteger("Start", 0);
        _sharkAnimator.SetInteger("Turn", 0);
    }
	
	// Update is called once per frame
	void FixedUpdate()
    {
        GameDataManager.Uno.motorLocker();
        
        // Move
        Lvector = new Vector3(LlastPos.x - driverLeftHand.position.x, 0, LlastPos.z - driverLeftHand.position.z);
        Ldistance = Vector3.Distance(driverLeftHand.position, DistanceCalculator.position);
        Rvector = new Vector3(RlastPos.x - driverRightHand.position.x, 0, RlastPos.z - driverRightHand.position.z);
        Rdistance = Vector3.Distance(driverRightHand.position, DistanceCalculator.position);
        diveDirection = directionContorl.position - driver.position;
        if ((LLastDistance - Ldistance) > 0.005 || (RLastDistance - Rdistance) > 0.005)
            LRvector = Lvector + Rvector;
        else
            LRvector -= LRvector * 3f * Time.deltaTime;
        body_vector_angle = Vector3.Angle(new Vector3(diveDirection.x, 0, diveDirection.z), new Vector3(LRvector.x, 0, LRvector.z));
        //rotation
        Debug.DrawRay(driver.transform.position, diveDirection * 10, Color.red, 10);
        if ((Rvector.magnitude > 0.025f || Lvector.magnitude > 0.025f) && ((LLastDistance - Ldistance) > 0.005 || (RLastDistance - Rdistance) > 0.005))
        {
            if ((Lvector.magnitude - Rvector.magnitude) > 0.025f) //trun right
            {
                if (rotateValue.magnitude <= 25)
                    rotateValue += Vector3.up * body_vector_angle * 0.16f;
                //Debug.LogWarning("Right!!");
                if (rotateValue.magnitude > 25) rotateValue = new Vector3(0, 25, 0);
            }
            else if ((Rvector.magnitude - Lvector.magnitude) > 0.025f)//turn left
            {
                if (rotateValue.magnitude <= 25)
                    rotateValue += Vector3.down * body_vector_angle * 0.16f;
                //Debug.LogWarning("Left!!");
                if (rotateValue.magnitude > 25) rotateValue = new Vector3(0, -25, 0);
            }
            else if (Mathf.Abs(Rvector.magnitude - Lvector.magnitude) < 0.02f)
            {
                //Debug.LogWarning("Foward!!");
            }
            
        }

        //dive Foward offset control
        if (LRvector.magnitude > 0.02f && LRvector.magnitude < 0.03f) offset += 0.06f;
        else if (LRvector.magnitude > 0.03f && LRvector.magnitude < 0.04f) offset += 0.055f;
        else if (LRvector.magnitude> 0.04f && LRvector.magnitude < 0.05f) offset += 0.05f;
        else if (LRvector.magnitude > 0.05f) offset += 0.055f;
        if (offset > 6) offset = 6;

        //foward and rotation
        transform.position += new Vector3(diveDirection.x, 0, diveDirection.z) * (LRvector.magnitude + offset) * 3f * Time.deltaTime;
        transform.Rotate(rotateValue * Time.deltaTime * (Time.deltaTime * 25));
        rotateValue -= rotateValue * Time.deltaTime * (Time.deltaTime * 50);
        //Debug.Log(LRvector.magnitude + offset + "rotateValue" + (rotateValue.y > 0));
        i = motorAngle(LRvector.magnitude + offset, rotateValue.y);
        Debug.Log(i.leftAngle + " " + i.rightAngle);
        new Thread(GameDataManager.Uno.sendData).Start(Left_degreeConvertToRotaryCoder((int)i.leftAngle) + " 255 " + Right_degreeConvertToRotaryCoder((int)i.rightAngle) + " 255");
        //reset parameter
        LlastPos = driverLeftHand.position;
        RlastPos = driverRightHand.position;
        LLastDistance = Ldistance;
        RLastDistance = Rdistance;
        Lvector = Vector3.zero;
        Rvector = Vector3.zero;
        if (rotateValue.magnitude <= 1)// 為了讓旋轉的值可以很快歸零，因為要讓它不要一值有殘餘的值
            rotateValue = Vector3.zero;
        if (offset > 0.0105625) offset -= 0.0105625f;
        else
        {
            offset = 0;
            LRvector = Vector3.zero;
        }
        // Shark
        if (Input.GetKeyDown(KeyCode.S) && _sharkAnimator.GetBool("On"))
        {
            specialEffectOn = true;
            Debug.Log("Press\"S\" for shark");
            randomTurn = UnityEngine.Random.Range(1, 3);
            _sharkAnimator.SetBool("On", false);
            _sharkAnimator.SetInteger("Start", 1);
            _sharkAnimator.SetInteger("Turn", randomTurn);
            shark.transform.localPosition = new Vector3(sharkShow.position.x, 0.58f, sharkShow.position.z);//new Vector3(-0.82f, 0.58f, -40f);
            shark.transform.localRotation = Quaternion.EulerRotation(0f, 0f, 0f);
        }
        
        if (_sharkAnimator.GetCurrentAnimatorStateInfo(0).IsName("Swiming"))
        {
            if (isShark)
            {
                isShark = false;
                _sharkAnimator.SetInteger("Start", 0);
            }
            else if (Vector3.Distance(shark.position, this.transform.position) < 20)
            {
                _sharkAnimator.SetBool("On", true);
            }
        }
        else if (_sharkAnimator.GetCurrentAnimatorStateInfo(0).IsName("Turn Left"))// Trun = 2
        {
            if (!isShark)
            {
                Debug.Log("shark_right");
            }
            isShark = true;
        }
        else if (_sharkAnimator.GetCurrentAnimatorStateInfo(0).IsName("Turn Right"))// Trun = 1
        {
            if (!isShark)
            {
                Debug.Log("shark_left");
            }
            isShark = true;
        }

        //FishFlock
        if (Input.GetKeyDown(KeyCode.F))
        {
            StartCoroutine(fishflock());
        }
    }

    private angle motorAngle (float fowardValue, float rotateValue)
    {
        angle answer = new angle();
        if (rotateValue > 15f) // right
        {
            Debug.Log("right");
            if (fowardValue >= 0 && fowardValue <= 1.5)  //0~1.5 20
            {
                answer.rightAngle = 20;
                answer.leftAngle = 20;
            }
            else if (fowardValue > 1.5 && fowardValue <= 5)//1.5~5 100
            {
                answer.rightAngle = 20;
                answer.leftAngle = 60;
            }
            else if (fowardValue > 5)//5up 130
            {
                answer.rightAngle = 60;
                answer.leftAngle = 130;
            }
        }
        else if (rotateValue < -15f) //left
        {
            Debug.Log("left");
            if (fowardValue >= 0 && fowardValue <= 1.5)  //0~1.5 20
            {
                answer.rightAngle = 20;
                answer.leftAngle = 20;
            }
            else if (fowardValue > 1.5 && fowardValue <= 5)//1.5~5 100
            {
                answer.rightAngle = 60;
                answer.leftAngle = 20;
            }
            else if (fowardValue > 5)//5up 130
            {
                answer.rightAngle = 130;
                answer.leftAngle = 60;
            }
        }
        else // foward
        {
            Debug.Log("foward");
            if (fowardValue >= 0 && fowardValue <= 1.5)  //0~1.5 20
            {
                answer.rightAngle = 20;
                answer.leftAngle = 20;
            }
            else if (fowardValue > 1.5 && fowardValue <= 5)//1.5~5 100
            {
                answer.rightAngle = 70;
                answer.leftAngle = 70;
            }
            else if (fowardValue > 5)//5up 130
            {
                answer.rightAngle = 130;
                answer.leftAngle = 130;
            }
        }

        return answer;
    }

    IEnumerator fishflock()
    {
        specialEffectOn = true;
        fishflockFlowControl.target = fishflockOn;
        yield return new WaitForSeconds(20f);
        fishflockFlowControl.target = fishflockOff;
    }
}
