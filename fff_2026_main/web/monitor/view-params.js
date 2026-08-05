export const viewParams = {
    fieldOfView: (80 * Math.PI) / 180,
    gridZ: 0,
    transform: [
        {
            //move 2 units back
            type:'translate',
            amount: [0,0,-2]
        },

        // {
        //     // look from above
        //     type:'rotate',
        //     amount: (_t)=>90 * Math.PI / 180,
        //     vector:[1,0,0]
        // },

        {
            // look slightly up
            type:'rotate',
            amount: (_t)=>0.3,
            vector:[1,0,0]
        },

        {
            //rotate the scene
            type:'rotate',
            amount: (t)=>t/8,
            vector: [0,1,0]
        },
        {
            // orient xy plane to be horizontal
            type:'rotate',
            amount: (_t)=>-3.14159/2,
            vector:[1,0,0]
        },
        {
            //move the camera to 2 meters above the floor
            type:'translate',
            amount: [0,0, 2 / -4.1]
        },
    ]
}