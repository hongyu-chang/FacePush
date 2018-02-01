﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using DG.Tweening;

public class hitted : MonoBehaviour {

    int s = 0;
    int state = 0;

    // Use this for initialization
    void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        //get animater in which state
        if (s != anim_change.s)
        {
            s = anim_change.s;
            if(anim_change.s != 5) state = anim_change.s;
        }

        //when hitted
        float time = 0.5f;
        float l = 0.7f;
        float r = 0.7f;

        if(collider_dir.Rhit == 1)
        {
            
            if (state == 1)
            {
                //moving position
                time = 0.2f;
                l = 1f;
                //moving rotation
                r = 10f;
            }
            else if (state == 3)
            {
                //moving position
                time = 0.4f;
                l = 0.7f;
                //moving rotation
                r = 5f;
            }

            Vector3 pos = this.transform.position + collider_dir.Rdir * l;
            Sequence mySequence = DOTween.Sequence(); 
            
            Tweener move1 = transform.DOMove(pos, time, true);
            Tweener rot1 = transform.DORotate(this.transform.rotation.eulerAngles + new Vector3(r, 0, r), 0.2f);
            Tweener move2 = transform.DOMove(this.transform.position, 0.5f);
            Tweener rot2 = transform.DORotate(this.transform.rotation.eulerAngles, 0.2f);

            mySequence.Append(move1);
            mySequence.Join(rot1);
            mySequence.Append(move2);
            mySequence.Join(rot2);
            Debug.Log("Rhit ");
            collider_dir.Rhit = 0;
        }
        else if (collider_dir.Lhit == 1)
        {
            if (state == 2)
            {
                //moving position
                time = 0.2f;
                l = 1.2f;
                //moving rotation
                r = 10f;
            }
            else if (state == 4)
            {
                //moving position
                time = 0.5f;
                l = 1f;
                //moving rotation
                r = 5f;
            }

            Vector3 pos = this.transform.position + collider_dir.Ldir * l;
            Sequence mySequence = DOTween.Sequence();
            
            Tweener move1 = transform.DOMove(pos, time, true);
            Tweener rot1 = transform.DORotate(this.transform.rotation.eulerAngles + new Vector3(r, 0, -r), 0.2f);
            Tweener move2 = transform.DOMove(this.transform.position, 0.5f);
            Tweener rot2 = transform.DORotate(this.transform.rotation.eulerAngles, 0.2f);

            mySequence.Append(move1);
            mySequence.Join(rot1);
            mySequence.Append(move2);
            mySequence.Join(rot2);
            Debug.Log("Lhit ");
            collider_dir.Lhit = 0;
        }
    }
}